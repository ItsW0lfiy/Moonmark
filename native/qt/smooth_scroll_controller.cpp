#include "smooth_scroll_controller.h"

#include <QWindow>

#include <algorithm>
#include <cmath>

namespace moonmark::qt {
namespace {
constexpr double response_rate = 17.0;
constexpr double stopped_velocity = 5.0;
constexpr double stopped_distance = 0.45;
}

SmoothScrollController::SmoothScrollController(QAbstractSlider* slider)
    : slider_(slider) {
    timer_.setInterval(16);
    timer_.setTimerType(Qt::PreciseTimer);
    QObject::connect(&timer_, &QTimer::timeout, &timer_, [this] { tick(); });
    if (slider_) {
        position_ = slider_->value();
        target_ = position_;
        QObject::connect(slider_, &QAbstractSlider::sliderPressed, &timer_,
                         [this] { cancel(); });
        QObject::connect(slider_, &QObject::destroyed, &timer_, [this] {
            timer_.stop();
            slider_ = nullptr;
            running_ = false;
        });
    }
}

void SmoothScrollController::addWheelDistance(double distance) {
    if (!slider_ || distance == 0.0) return;
    const double minimum = slider_->minimum();
    const double maximum = slider_->maximum();
    if (!running_) {
        position_ = slider_->value();
        target_ = position_;
        velocity_ = 0.0;
        frame_values_.clear();
        frame_values_.push_back(slider_->value());
        first_change_us_ = -1;
        request_elapsed_.restart();
        elapsed_.restart();
        running_ = true;
    }
    target_ = std::clamp(target_ + distance, minimum, maximum);
    requestFrame();
}

void SmoothScrollController::moveDirectlyTo(int destination) {
    if (!slider_) return;
    cancel();
    const int value = std::clamp(destination, slider_->minimum(), slider_->maximum());
    position_ = value;
    target_ = value;
    slider_->setValue(value);
}

void SmoothScrollController::cancel() {
    timer_.stop();
    running_ = false;
    velocity_ = 0.0;
    if (slider_) {
        position_ = slider_->value();
        target_ = position_;
    }
}

bool SmoothScrollController::isRunning() const { return running_; }

int SmoothScrollController::targetValue() const {
    return static_cast<int>(std::lround(target_));
}

double SmoothScrollController::velocity() const { return velocity_; }

qint64 SmoothScrollController::firstChangeMicros() const { return first_change_us_; }

const QVector<int>& SmoothScrollController::frameValues() const { return frame_values_; }

void SmoothScrollController::requestFrame() {
    if (!running_ || timer_.isActive()) return;
    // QWidget raster backing stores do not expose a reliable presented-frame
    // callback. A precise 60 Hz timer is the practical cadence source; every
    // scrollbar write still requests a real viewport update and elapsed time,
    // not frame count, determines motion state.
    timer_.start();
}

void SmoothScrollController::tick() {
    if (!running_ || !slider_) {
        cancel();
        return;
    }
    const qint64 elapsed_ns = elapsed_.nsecsElapsed();
    elapsed_.restart();
    const double dt = std::max(0.000001, static_cast<double>(elapsed_ns) / 1'000'000'000.0);

    // Exact critically damped integration for a fixed target over dt. This is
    // stable across variable frame intervals and never imposes a px/frame cap.
    const double displacement = position_ - target_;
    const double c = velocity_ + response_rate * displacement;
    const double decay = std::exp(-response_rate * dt);
    position_ = target_ + (displacement + c * dt) * decay;
    velocity_ = (velocity_ - response_rate * c * dt) * decay;

    if (frame_sampled) frame_sampled(dt, position_, velocity_);
    int next = std::clamp(static_cast<int>(std::lround(position_)), slider_->minimum(),
                          slider_->maximum());
    if (std::abs(target_ - position_) <= stopped_distance &&
        std::abs(velocity_) <= stopped_velocity) {
        position_ = target_;
        velocity_ = 0.0;
        next = targetValue();
    }
    if (next != slider_->value()) {
        QElapsedTimer write;
        write.start();
        slider_->setValue(next);
        if (scrollbar_write_measured) scrollbar_write_measured(write.nsecsElapsed() / 1000);
        frame_values_.push_back(next);
        if (first_change_us_ < 0) first_change_us_ = request_elapsed_.nsecsElapsed() / 1000;
        if (value_changed) value_changed(next);
    }
    if (position_ == target_ && velocity_ == 0.0) {
        finish();
    } else {
        if (auto* window = slider_->window() ? slider_->window()->windowHandle() : nullptr)
            window->requestUpdate();
    }
}

void SmoothScrollController::finish() {
    timer_.stop();
    running_ = false;
    if (finished) finished();
}

} // namespace moonmark::qt
