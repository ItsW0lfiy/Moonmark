#pragma once

#include <QAbstractSlider>
#include <QEasingCurve>
#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>
#include <QVector>
#include <functional>

namespace moonmark::qt {

// Elapsed-time scroll interpolation shared by document and navigation surfaces.
// The controller owns no document semantics and never queues animations.
class SmoothScrollController final {
public:
    explicit SmoothScrollController(QAbstractSlider* slider);

    void animateTo(int destination, int duration_ms,
                   QEasingCurve::Type easing = QEasingCurve::InOutCubic);
    void retargetTo(int destination, int duration_ms,
                    QEasingCurve::Type easing = QEasingCurve::InOutCubic);
    void cancel();

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] int targetValue() const;
    [[nodiscard]] qint64 firstChangeMicros() const;
    [[nodiscard]] const QVector<int>& frameValues() const;

    std::function<void(int)> value_changed;
    std::function<void()> finished;

private:
    void begin(int destination, int duration_ms, QEasingCurve::Type easing,
               bool reset_measurements);
    void tick();

    QPointer<QAbstractSlider> slider_;
    QTimer timer_;
    QElapsedTimer elapsed_;
    QElapsedTimer request_elapsed_;
    QEasingCurve easing_ = QEasingCurve::InOutCubic;
    QVector<int> frame_values_;
    int start_value_ = 0;
    int target_value_ = 0;
    int duration_ms_ = 0;
    int maximum_step_ = 28;
    qint64 first_change_us_ = -1;
};

} // namespace moonmark::qt
