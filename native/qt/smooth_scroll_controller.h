#pragma once

#include <QAbstractSlider>
#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>
#include <QVector>
#include <functional>

namespace moonmark::qt {

// Stateful, time-based wheel motion. Input updates the current trajectory rather
// than restarting an easing curve. Floating-point state is retained until the
// final scrollbar write so fast motion is not quantized into fixed pixel steps.
class SmoothScrollController final {
public:
    explicit SmoothScrollController(QAbstractSlider* slider);

    void addWheelDistance(double distance);
    void moveDirectlyTo(int destination);
    void cancel();

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] int targetValue() const;
    [[nodiscard]] double velocity() const;
    [[nodiscard]] qint64 firstChangeMicros() const;
    [[nodiscard]] const QVector<int>& frameValues() const;

    std::function<void(int)> value_changed;
    std::function<void()> finished;
    std::function<void(double, double, double)> frame_sampled;
    std::function<void(qint64)> scrollbar_write_measured;

private:
    void requestFrame();
    void tick();
    void finish();

    QPointer<QAbstractSlider> slider_;
    QTimer timer_;
    QElapsedTimer elapsed_;
    QElapsedTimer request_elapsed_;
    QVector<int> frame_values_;
    double position_ = 0.0;
    double target_ = 0.0;
    double velocity_ = 0.0;
    bool running_ = false;
    qint64 first_change_us_ = -1;
};

} // namespace moonmark::qt
