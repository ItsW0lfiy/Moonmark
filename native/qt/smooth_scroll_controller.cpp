#include "smooth_scroll_controller.h"

#include <algorithm>
#include <cmath>

namespace moonmark::qt {

SmoothScrollController::SmoothScrollController(QAbstractSlider* slider) : slider_(slider) {
    // One logical update per normal display frame. Together with the bounded
    // step below this prevents event-loop stalls or long journeys from turning
    // a nominal animation into a handful of large scrollbar teleports.
    timer_.setInterval(16);
    timer_.setTimerType(Qt::PreciseTimer);
    QObject::connect(&timer_, &QTimer::timeout, &timer_, [this] { tick(); });
    if (slider_) {
        QObject::connect(slider_, &QAbstractSlider::sliderPressed, &timer_, [this] { cancel(); });
        QObject::connect(slider_, &QObject::destroyed, &timer_, [this] {
            timer_.stop();
            slider_ = nullptr;
        });
    }
}

void SmoothScrollController::animateTo(int destination, int duration_ms,
                                       QEasingCurve::Type easing) {
    begin(destination, duration_ms, easing, true);
}

void SmoothScrollController::retargetTo(int destination, int duration_ms,
                                        QEasingCurve::Type easing) {
    begin(destination, duration_ms, easing, false);
}

void SmoothScrollController::begin(int destination, int duration_ms,
                                   QEasingCurve::Type easing, bool reset_measurements) {
    if (!slider_) return;
    destination = std::clamp(destination, slider_->minimum(), slider_->maximum());
    if (reset_measurements) {
        frame_values_.clear();
        first_change_us_ = -1;
        request_elapsed_.restart();
        frame_values_.push_back(slider_->value());
    }
    timer_.stop();
    start_value_ = slider_->value();
    target_value_ = destination;
    duration_ms_ = std::max(0, duration_ms);
    easing_ = QEasingCurve(easing);
    if (start_value_ == target_value_) {
        if (finished) finished();
        return;
    }
    if (duration_ms_ == 0) {
        slider_->setValue(target_value_);
        frame_values_.push_back(target_value_);
        if (first_change_us_ < 0) first_change_us_ = request_elapsed_.nsecsElapsed() / 1000;
        if (value_changed) value_changed(target_value_);
        if (finished) finished();
        return;
    }
    elapsed_.restart();
    timer_.start();
}

void SmoothScrollController::cancel() {
    timer_.stop();
    if (slider_) target_value_ = slider_->value();
}

bool SmoothScrollController::isRunning() const {
    return timer_.isActive();
}

int SmoothScrollController::targetValue() const {
    return target_value_;
}

qint64 SmoothScrollController::firstChangeMicros() const {
    return first_change_us_;
}

const QVector<int>& SmoothScrollController::frameValues() const {
    return frame_values_;
}

void SmoothScrollController::tick() {
    if (!slider_) {
        timer_.stop();
        return;
    }
    const qreal progress = std::clamp(static_cast<qreal>(elapsed_.elapsed()) /
                                         static_cast<qreal>(duration_ms_),
                                     0.0, 1.0);
    const qreal eased = easing_.valueForProgress(progress);
    const int desired = progress >= 1.0
        ? target_value_
        : static_cast<int>(std::round(start_value_ + (target_value_ - start_value_) * eased));
    const int remaining = desired - slider_->value();
    const int next = slider_->value() + std::clamp(remaining, -maximum_step_, maximum_step_);
    if (next != slider_->value()) {
        slider_->setValue(next);
        frame_values_.push_back(next);
        if (first_change_us_ < 0) first_change_us_ = request_elapsed_.nsecsElapsed() / 1000;
        if (value_changed) value_changed(next);
    }
    if (next == target_value_) {
        timer_.stop();
        if (finished) finished();
    }
}

} // namespace moonmark::qt
