#ifndef LANIMATION_H
#define LANIMATION_H

#include <LObject.h>
#include <SRM/SRMTypes.h>

#include <chrono>
#include <functional>

#include "LNamespaces.h"

/**
 * @brief Time-based animations.
 *
 * This class can be used for animating object parameters such as positions,
 * color, opacity, etc. It has a fixed duration in milliseconds, and is
 * synchronized with each initialized output's render loop.\n After started, the
 * `onUpdate()` callback is triggered before each LOutput::paintGL() call,
 * allowing you to access the value() property, which is a 64-bit floating-point
 * number linearly interpolated from 0.0 to 1.0, indicating the completion
 * percentage of the animation.
 *
 * @note It is essential to manually invoke LOutput::repaint() on the outputs
 * you are animating; otherwise, the `onUpdate()` callback may not be invoked.
 *
 * After the animation finishes, the `onFinish()` callback is triggered, and the
 * value() property has a value of 1.0.\n
 */
class Louvre::LAnimation : public LObject {
 public:
  /**
   * Callback function type used to handle the `onUpdate()` and `onFinish()`
   * events.
   */
  using Callback = std::function<void(LAnimation *)>;

  /**
   * @brief Creates a reusable animation.
   *
   * Creates an animation without starting it immediately.
   *
   * @param durationMs The duration of the animation in milliseconds.
   * @param onUpdate A callback function triggered each time the value()
   * property changes. `nullptr` can be passed if not used.
   * @param onFinish A callback function triggered once the value() property
   * reaches 1.0. `nullptr` can be passed if not used.
   */
  LAnimation(UInt32 durationMs = 0, const Callback &onUpdate = nullptr,
             const Callback &onFinish = nullptr) noexcept;

  /**
   * @brief Destructor for the LAnimation class.
   *
   * Destroys an animation object. If the animation is currently running at the
   * time of destruction, the `onFinish()` callback is invoked immediately
   * before the object is destroyed.
   */
  ~LAnimation();

  LCLASS_NO_COPY(LAnimation)

  /**
   * @brief Creates and launches a one-time animation with automatic cleanup.
   *
   * The oneShot() method creates and starts an animation immediately, and it is
   * automatically destroyed once finished.
   *
   * @param durationMs The duration of the animation in milliseconds.
   * @param onUpdate A callback function triggered each time the value()
   * property changes. `nullptr` can be passed if not used.
   * @param onFinish A callback function triggered once the value() property
   * reaches 1.0. `nullptr` can be passed if not used.
   */
  static void oneShot(UInt32 durationMs, const Callback &onUpdate = nullptr,
                      const Callback &onFinish = nullptr) noexcept;

  /**
   * @brief Sets the `onUpdate()` callback handler function.
   *
   * This method allows you to set the callback function that will be called
   * when an update event occurs.
   *
   * @param onUpdate A reference to the callback function. Pass `nullptr` to
   * disable the callback.
   */
  void setOnUpdateCallback(const Callback &onUpdate) noexcept;

  /**
   * @brief Sets the `onFinish()` callback handler function.
   *
   * This method allows you to set the callback function that will be called
   * when the animaion finishes or stop() is called.
   *
   * @param onFinish A reference to the callback function. Pass `nullptr` to
   * disable the callback.
   */
  void setOnFinishCallback(const Callback &onFinish) noexcept;

  /**
   * @brief Sets the duration of the animation in milliseconds.
   *
   * Use this method to specify the duration of the animation in milliseconds.
   *
   * @note It is not permissible to invoke this method while the animation is in
   * progress, and attempting to do so will yield no results.
   *
   * @param durationMs The duration of the animation in milliseconds.
   */
  void setDuration(UInt32 durationMs) noexcept {
    if (m_running) return;

    m_duration = durationMs;
  }

  /**
   * @brief Returns the duration of the animation in milliseconds.
   *
   * Use this method to retrieve the duration of the animation in milliseconds.
   *
   * @return The duration of the animation in milliseconds.
   */
  UInt32 duration() const noexcept { return m_duration; }

  /**
   * @brief Returns a number linearly interpolated from 0.0 to 1.0.
   *
   * This method returns a value indicating the percentage of completion of the
   * animation. The value is linearly interpolated between 0.0 (start of the
   * animation) and 1.0 (end of the animation).
   *
   * @return The interpolated completion value ranging from 0.0 to 1.0.
   */
  Float64 value() const noexcept { return m_value; }

  /**
   * @brief Returns a value interpolated between two given numbers.
   *
   * Calculates the interpolated value between `begin` and `end`
   * based on the current progress of the animation.
   *
   * @param begin The value representing the start of the range.
   * @param end   The value representing the end of the range.
   * @return The interpolated value, ranging from `begin` (when progress is 0)
   *         to `end` (when progress equals the duration).
   */
  Float64 value(Float64 begin, Float64 end) {
    Float64 t = value();
    Float64 c = end - begin;
    return c * t + begin;
  }

  Float64 inQuad(Float64 begin, Float64 end) {
    Float64 t = value();
    Float64 c = end - begin;
    return c * t * t + begin;
  }

  Float64 outQuad(Float64 begin, Float64 end) {
    Float64 c = end - begin;
    Float64 t = value();
    return -c * t * (t - 2) + begin;
  }

  Float64 inOutQuad(Float64 begin, Float64 end) {
    Float64 c = end - begin;
    Float64 t = value() * 2.0;
    if (t < 1)
      return c / 2 * t * t + begin;
    else
      return -c / 2 * ((t - 1) * (t - 3) - 1) + begin;
  }

  Float64 outInQuad(Float64 begin, Float64 end) {
    Float64 c = end - begin;
    Float64 t = value() * duration();
    Float64 d = duration();

    if (t < d / 2.0) {
      t = t * 2 / d;
      return -(c / 2.0) * t * (t - 2) + begin;
    } else {
      t = (t * 2 - d) / d;
      return (c / 2.0) * t * t + begin + c / 2.0;
    }
  }

  double inCubic(double begin, double end) {
    double c = end - begin;
    double t = value();
    return c * t * t * t + begin;
  }

  double outCubic(double begin, double end) {
    double c = end - begin;
    double t = value() - 1.0;
    return c * (t * t * t + 1.0) + begin;
  }

  double inOutCubic(double begin, double end) {
    double c = end - begin;
    double t = value() * 2.0;
    if (t < 1.0) {
      return c / 2.0 * t * t * t + begin;
    } else {
      t = t - 2.0;
      return c / 2.0 * (t * t * t + 2.0) + begin;
    }
  }

  double outInCubic(double begin, double end) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d - 1.0;
      return c / 2.0 * (t * t * t + 1.0) + begin;
    } else {
      t = (t * 2.0 - d) / d;
      return c / 2.0 * t * t * t + begin + c / 2.0;
    }
  }

  double inQuart(double begin, double end) {
    double c = end - begin;
    double t = value();
    return c * std::pow(t, 4) + begin;
  }

  double outQuart(double begin, double end) {
    double c = end - begin;
    double t = value() - 1.0;
    return -c * (std::pow(t, 4) - 1.0) + begin;
  }

  double inOutQuart(double begin, double end) {
    double c = end - begin;
    double t = value() * 2.0;
    if (t < 1.0) {
      return c / 2.0 * std::pow(t, 4) + begin;
    } else {
      t = t - 2.0;
      return -c / 2.0 * (std::pow(t, 4) - 2.0) + begin;
    }
  }

  double outInQuart(double begin, double end) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d - 1.0;
      return -c / 2.0 * (std::pow(t, 4) - 1.0) + begin;
    } else {
      t = (t * 2.0 - d) / d;
      return c / 2.0 * std::pow(t, 4) + begin + c / 2.0;
    }
  }

  double inQuint(double begin, double end) {
    double c = end - begin;
    double t = value();
    return c * std::pow(t, 5) + begin;
  }

  double outQuint(double begin, double end) {
    double c = end - begin;
    double t = value() - 1.0;
    return c * (std::pow(t, 5) + 1.0) + begin;
  }

  double inOutQuint(double begin, double end) {
    double c = end - begin;
    double t = value() * 2.0;
    if (t < 1.0) {
      return c / 2.0 * std::pow(t, 5) + begin;
    } else {
      t = t - 2.0;
      return c / 2.0 * (std::pow(t, 5) + 2.0) + begin;
    }
  }

  double outInQuint(double begin, double end) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d - 1.0;
      return c / 2.0 * (std::pow(t, 5) + 1.0) + begin;
    } else {
      t = (t * 2.0 - d) / d;
      return c / 2.0 * std::pow(t, 5) + begin + c / 2.0;
    }
  }

  double inSine(double begin, double end) {
    double c = end - begin;
    double t = value();
    return -c * std::cos(t * (M_PI / 2.0)) + c + begin;
  }

  double outSine(double begin, double end) {
    double c = end - begin;
    double t = value();
    return c * std::sin(t * (M_PI / 2.0)) + begin;
  }

  double inOutSine(double begin, double end) {
    double c = end - begin;
    double t = value();
    return -c / 2.0 * (std::cos(M_PI * t) - 1.0) + begin;
  }

  double outInSine(double begin, double end) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d;
      return c / 2.0 * std::sin(t * (M_PI / 2.0)) + begin;
    } else {
      t = (t * 2.0 - d) / d;
      return -c / 2.0 * std::cos(t * (M_PI / 2.0)) + c / 2.0 + begin + c / 2.0;
    }
  }

  double inExpo(double begin, double end) {
    double c = end - begin;
    double t = value();
    if (t == 0.0) return begin;
    return c * std::pow(2.0, 10.0 * (t - 1.0)) + begin - c * 0.001;
  }

  double outExpo(double begin, double end) {
    double c = end - begin;
    double t = value();
    if (t == 1.0) return begin + c;
    return c * 1.001 * (1.0 - std::pow(2.0, -10.0 * t)) + begin;
  }

  double inOutExpo(double begin, double end) {
    double c = end - begin;
    double t = value() * 2.0;
    if (t == 0.0) return begin;
    if (t == 2.0) return begin + c;
    if (t < 1.0) {
      return c / 2.0 * std::pow(2.0, 10.0 * (t - 1.0)) + begin - c * 0.0005;
    } else {
      t = t - 1.0;
      return c / 2.0 * 1.0005 * (2.0 - std::pow(2.0, -10.0 * t)) + begin;
    }
  }

  double outInExpo(double begin, double end) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d;
      if (t == 1.0) return begin + c / 2.0;
      return c / 2.0 * 1.001 * (1.0 - std::pow(2.0, -10.0 * t)) + begin;
    } else {
      t = (t * 2.0 - d) / d;
      if (t == 0.0) return begin + c / 2.0;
      return c / 2.0 * std::pow(2.0, 10.0 * (t - 1.0)) + begin + c / 2.0 -
             c / 2.0 * 0.001;
    }
  }

  double inCirc(double begin, double end) {
    double c = end - begin;
    double t = value();
    return -c * (std::sqrt(1.0 - t * t) - 1.0) + begin;
  }

  double outCirc(double begin, double end) {
    double c = end - begin;
    double t = value() - 1.0;
    return c * std::sqrt(1.0 - t * t) + begin;
  }

  double inOutCirc(double begin, double end) {
    double c = end - begin;
    double t = value() * 2.0;
    if (t < 1.0) {
      return -c / 2.0 * (std::sqrt(1.0 - t * t) - 1.0) + begin;
    } else {
      t = t - 2.0;
      return c / 2.0 * (std::sqrt(1.0 - t * t) + 1.0) + begin;
    }
  }

  double outInCirc(double begin, double end) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d - 1.0;
      return c / 2.0 * std::sqrt(1.0 - t * t) + begin;
    } else {
      t = (t * 2.0 - d) / d;
      return -c / 2.0 * (std::sqrt(1.0 - t * t) - 1.0) + begin + c / 2.0;
    }
  }

  double inElastic(double begin, double end, double a = 0.0, double p = 0.0) {
    double c = end - begin;
    double t = value();
    if (t == 0.0) return begin;
    if (t == 1.0) return begin + c;
    if (p == 0.0) p = duration() * 0.3;
    double s;
    if (a == 0.0 || a < std::abs(c)) {
      a = c;
      s = p / 4.0;
    } else {
      s = p / (2.0 * M_PI) * std::asin(c / a);
    }
    t = t - 1.0;
    return -(a * std::pow(2.0, 10.0 * t) *
             std::sin((t * duration() - s) * (2.0 * M_PI) / p)) +
           begin;
  }

  double outElastic(double begin, double end, double a = 0.0, double p = 0.0) {
    double c = end - begin;
    double t = value();
    if (t == 0.0) return begin;
    if (t == 1.0) return begin + c;
    if (p == 0.0) p = duration() * 0.3;
    double s;
    if (a == 0.0 || a < std::abs(c)) {
      a = c;
      s = p / 4.0;
    } else {
      s = p / (2.0 * M_PI) * std::asin(c / a);
    }
    return a * std::pow(2.0, -10.0 * t) *
               std::sin((t * duration() - s) * (2.0 * M_PI) / p) +
           c + begin;
  }

  double inOutElastic(double begin, double end, double a = 0.0,
                      double p = 0.0) {
    double c = end - begin;
    double t = value() * 2.0;
    if (t == 0.0) return begin;
    if (t == 2.0) return begin + c;
    if (p == 0.0) p = duration() * 0.3 * 1.5;
    if (a == 0.0) a = 0.0;
    double s;
    if (a == 0.0 || a < std::abs(c)) {
      a = c;
      s = p / 4.0;
    } else {
      s = p / (2.0 * M_PI) * std::asin(c / a);
    }
    if (t < 1.0) {
      t = t - 1.0;
      return -0.5 * (a * std::pow(2.0, 10.0 * t) *
                     std::sin((t * duration() - s) * (2.0 * M_PI) / p)) +
             begin;
    } else {
      t = t - 1.0;
      return a * std::pow(2.0, -10.0 * t) *
                 std::sin((t * duration() - s) * (2.0 * M_PI) / p) * 0.5 +
             c + begin;
    }
  }

  double outInElastic(double begin, double end, double a = 0.0,
                      double p = 0.0) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d;
      if (t == 0.0) return begin;
      if (t == 1.0) return begin + c / 2.0;
      if (p == 0.0) p = d * 0.3;
      double s;
      if (a == 0.0 || a < std::abs(c / 2.0)) {
        a = c / 2.0;
        s = p / 4.0;
      } else {
        s = p / (2.0 * M_PI) * std::asin(c / 2.0 / a);
      }
      return a * std::pow(2.0, -10.0 * t) *
                 std::sin((t * d - s) * (2.0 * M_PI) / p) +
             c / 2.0 + begin;
    } else {
      t = (t * 2.0 - d) / d;
      if (t == 0.0) return begin + c / 2.0;
      if (t == 1.0) return begin + c;
      if (p == 0.0) p = d * 0.3;
      double s;
      if (a == 0.0 || a < std::abs(c / 2.0)) {
        a = c / 2.0;
        s = p / 4.0;
      } else {
        s = p / (2.0 * M_PI) * std::asin(c / 2.0 / a);
      }
      t = t - 1.0;
      return -(a * std::pow(2.0, 10.0 * t) *
               std::sin((t * d - s) * (2.0 * M_PI) / p)) +
             begin + c / 2.0;
    }
  }

  double inBack(double begin, double end, double s = 1.70158) {
    double c = end - begin;
    double t = value();
    return c * t * t * ((s + 1.0) * t - s) + begin;
  }

  double outBack(double begin, double end, double s = 1.70158) {
    double c = end - begin;
    double t = value() - 1.0;
    return c * (t * t * ((s + 1.0) * t + s) + 1.0) + begin;
  }

  double inOutBack(double begin, double end, double s = 1.70158) {
    double c = end - begin;
    s = s * 1.525;
    double t = value() * 2.0;
    if (t < 1.0) {
      return c / 2.0 * (t * t * ((s + 1.0) * t - s)) + begin;
    } else {
      t = t - 2.0;
      return c / 2.0 * (t * t * ((s + 1.0) * t + s) + 2.0) + begin;
    }
  }

  double outInBack(double begin, double end, double s = 1.70158) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d - 1.0;
      return c / 2.0 * (t * t * ((s + 1.0) * t + s) + 1.0) + begin;
    } else {
      t = (t * 2.0 - d) / d;
      return c / 2.0 * t * t * ((s + 1.0) * t - s) + begin + c / 2.0;
    }
  }

  double outBounce(double begin, double end) {
    double c = end - begin;
    double t = value();
    if (t < 1.0 / 2.75) {
      return c * (7.5625 * t * t) + begin;
    } else if (t < 2.0 / 2.75) {
      t = t - (1.5 / 2.75);
      return c * (7.5625 * t * t + 0.75) + begin;
    } else if (t < 2.5 / 2.75) {
      t = t - (2.25 / 2.75);
      return c * (7.5625 * t * t + 0.9375) + begin;
    } else {
      t = t - (2.625 / 2.75);
      return c * (7.5625 * t * t + 0.984375) + begin;
    }
  }

  double inBounce(double begin, double end) {
    double c = end - begin;
    double t = value();
    double d = duration();
    t = 1.0 - t;
    if (t < 1.0 / 2.75) {
      return c - (c * (7.5625 * t * t)) + begin;
    } else if (t < 2.0 / 2.75) {
      t = t - (1.5 / 2.75);
      return c - (c * (7.5625 * t * t + 0.75)) + begin;
    } else if (t < 2.5 / 2.75) {
      t = t - (2.25 / 2.75);
      return c - (c * (7.5625 * t * t + 0.9375)) + begin;
    } else {
      t = t - (2.625 / 2.75);
      return c - (c * (7.5625 * t * t + 0.984375)) + begin;
    }
  }

  double inOutBounce(double begin, double end) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d;
      t = 1.0 - t;
      if (t < 1.0 / 2.75) {
        return c / 2.0 * (1.0 - (7.5625 * t * t)) + begin;
      } else if (t < 2.0 / 2.75) {
        t = t - (1.5 / 2.75);
        return c / 2.0 * (1.0 - (7.5625 * t * t + 0.75)) + begin;
      } else if (t < 2.5 / 2.75) {
        t = t - (2.25 / 2.75);
        return c / 2.0 * (1.0 - (7.5625 * t * t + 0.9375)) + begin;
      } else {
        t = t - (2.625 / 2.75);
        return c / 2.0 * (1.0 - (7.5625 * t * t + 0.984375)) + begin;
      }
    } else {
      t = (t * 2.0 - d) / d;
      if (t < 1.0 / 2.75) {
        return c / 2.0 * (7.5625 * t * t) + begin + c / 2.0;
      } else if (t < 2.0 / 2.75) {
        t = t - (1.5 / 2.75);
        return c / 2.0 * (7.5625 * t * t + 0.75) + begin + c / 2.0;
      } else if (t < 2.5 / 2.75) {
        t = t - (2.25 / 2.75);
        return c / 2.0 * (7.5625 * t * t + 0.9375) + begin + c / 2.0;
      } else {
        t = t - (2.625 / 2.75);
        return c / 2.0 * (7.5625 * t * t + 0.984375) + begin + c / 2.0;
      }
    }
  }
  double outInBounce(double begin, double end) {
    double c = end - begin;
    double t = value() * duration();
    double d = duration();
    if (t < d / 2.0) {
      t = t * 2.0 / d;
      if (t < 1.0 / 2.75) {
        return c / 2.0 * (7.5625 * t * t) + begin;
      } else if (t < 2.0 / 2.75) {
        t = t - (1.5 / 2.75);
        return c / 2.0 * (7.5625 * t * t + 0.75) + begin;
      } else if (t < 2.5 / 2.75) {
        t = t - (2.25 / 2.75);
        return c / 2.0 * (7.5625 * t * t + 0.9375) + begin;
      } else {
        t = t - (2.625 / 2.75);
        return c / 2.0 * (7.5625 * t * t + 0.984375) + begin;
      }
    } else {
      t = (t * 2.0 - d) / d;
      t = 1.0 - t;
      if (t < 1.0 / 2.75) {
        return c / 2.0 - (c / 2.0 * (7.5625 * t * t)) + begin + c / 2.0;
      } else if (t < 2.0 / 2.75) {
        t = t - (1.5 / 2.75);
        return c / 2.0 - (c / 2.0 * (7.5625 * t * t + 0.75)) + begin + c / 2.0;
      } else if (t < 2.5 / 2.75) {
        t = t - (2.25 / 2.75);
        return c / 2.0 - (c / 2.0 * (7.5625 * t * t + 0.9375)) + begin +
               c / 2.0;
      } else {
        t = t - (2.625 / 2.75);
        return c / 2.0 - (c / 2.0 * (7.5625 * t * t + 0.984375)) + begin +
               c / 2.0;
      }
    }
  }

  /**
   * @brief Starts the animation.
   *
   * If the animation is already running, calling this method a no-op.
   */
  void start() noexcept;

  /**
   * @brief Halts the animation before its duration is reached.
   *
   * The stop() method can be used to stop the animation before its duration is
   * reached. If called before the animation finishes, the `onFinish()` callback
   * is triggered immediately, and the value() property is set to 1.0.
   */
  void stop();

  /**
   * @brief Checks if the animation is currently running.
   *
   * @return `true` if running, `false` otherwise.
   */
  bool running() const noexcept { return m_running; }

 private:
  friend class LCompositor;
  Callback m_onUpdate{nullptr};
  Float64 m_value{0.0};
  Int64 m_duration;
  std::chrono::steady_clock::time_point m_beginTime;
  bool m_running{false};
  bool m_processed{false};
  bool m_pendingDestroy{false};
  bool m_destroyOnFinish{false};
  Callback m_onFinish{nullptr};
};

#endif  // LANIMATION_H
