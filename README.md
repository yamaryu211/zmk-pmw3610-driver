PMW3610 driver implementation for ZMK with at least Zephyr 3.5

This work is based on [ufan's implementation](https://github.com/ufan/zmk/tree/support-trackpad) of the driver.

## Installation

Only GitHub actions builds are covered here. Local builds are different for each user, therefore it's not possible to cover all cases.

Include this project on your ZMK's west manifest in `config/west.yml`:

```yml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/petejohanson
    - name: inorichi
      url-base: https://github.com/inorichi
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: feat/pointers-move-scroll
      import: app/west.yml
    - name: zmk-pmw3610-driver
      remote: inorichi
      revision: main
  self:
    path: config
```

Then, edit your `build.yml` to look like this, 3.5 is now on main:

```yml
on: [workflow_dispatch]

jobs:
  build:
    uses: zmkfirmware/zmk/.github/workflows/build-user-config.yml@main
```

Now, update your `board.overlay` adding the necessary bits (update the pins for your board accordingly):

```dts
&pinctrl {
    spi0_default: spi0_default {
        group1 {
            psels = <NRF_PSEL(SPIM_SCK, 0, 8)>,
                <NRF_PSEL(SPIM_MOSI, 0, 17)>,
                <NRF_PSEL(SPIM_MISO, 0, 17)>;
        };
    };

    spi0_sleep: spi0_sleep {
        group1 {
            psels = <NRF_PSEL(SPIM_SCK, 0, 8)>,
                <NRF_PSEL(SPIM_MOSI, 0, 17)>,
                <NRF_PSEL(SPIM_MISO, 0, 17)>;
            low-power-enable;
        };
    };
};

&spi0 {
    status = "okay";
    compatible = "nordic,nrf-spim";
    pinctrl-0 = <&spi0_default>;
    pinctrl-1 = <&spi0_sleep>;
    pinctrl-names = "default", "sleep";
    cs-gpios = <&gpio0 20 GPIO_ACTIVE_LOW>;

    trackball: trackball@0 {
        status = "okay";
        compatible = "pixart,pmw3610";
        reg = <0>;
        spi-max-frequency = <2000000>;
        irq-gpios = <&gpio0 6 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;

        /*   optional features   */
        // snipe-layers = <1>;
        // scroll-layers = <2 3>;
        // automouse-layer = <4>;

        /*   optional: ball action on specific layers  */
        // arrows {
        //     layers = <3>;
        //     bindings =
        //         <&kp RIGHT_ARROW>,
        //         <&kp LEFT_ARROW>,
        //         <&kp UP_ARROW>,
        //         <&kp DOWN_ARROW>;
        //
        //     /*   optional: ball action configuration  */
        //     tick = <10>;
        //     // wait-ms = <5>;
        //     // tap-ms = <5>;
        // };
    };
};

/ {
  trackball_listener {
    compatible = "zmk,input-listener";
    device = <&trackball>;

  };
};
```

Now enable the driver config in your `board.config` file (read the Kconfig file to find out all possible options):

```conf
CONFIG_SPI=y
CONFIG_INPUT=y
CONFIG_ZMK_MOUSE=y
CONFIG_PMW3610=y
```

## Mac Mouse Fix Style Smooth Scrolling

This driver includes an enhanced scrolling feature inspired by Mac Mouse Fix, providing a smooth and natural scrolling experience similar to macOS Magic Mouse or Mac Mouse Fix software.

### Key Features

1. **Momentum Scrolling**: After stopping scroll input, scrolling continues naturally with momentum and gradually decelerates
2. **Smooth Deceleration**: Exponential decay curve provides natural stopping feel
3. **Fine Scroll Control**: High-precision scroll division for ultra-smooth scrolling
4. **Adaptive Speed Control**: Momentum strength automatically adjusts based on input velocity

### Configuration

#### 1. Enable Mac Mouse Fix Style Scrolling

Add these options to your Kconfig:

```conf
CONFIG_PMW3610_SCROLL_MODE_STANDARD=n
CONFIG_PMW3610_SCROLL_MODE_MAC_MOUSE_FIX=y
```

#### 2. Fine-tune Parameters

Customize the scrolling behavior with these parameters:

```conf
# Friction coefficient (smaller = stops sooner)
CONFIG_PMW3610_MOMENTUM_SCROLL_FRICTION=980  # 0.98

# Minimum velocity for momentum scrolling (stops below this)
CONFIG_PMW3610_MOMENTUM_SCROLL_MIN_VELOCITY=50  # 0.5

# Initial momentum boost multiplier
CONFIG_PMW3610_MOMENTUM_SCROLL_BOOST=150  # 1.5x

# Maximum momentum duration (milliseconds)
CONFIG_PMW3610_MOMENTUM_SCROLL_TIMEOUT_MS=2000

# Smooth scroll division (higher = finer control)
CONFIG_PMW3610_SMOOTH_SCROLL_DIVIDER=3
```

### Parameter Details

- **MOMENTUM_SCROLL_FRICTION**: Controls momentum decay rate. 980 (0.98) provides smooth deceleration
- **MOMENTUM_SCROLL_MIN_VELOCITY**: Momentum stops when velocity drops below this threshold
- **MOMENTUM_SCROLL_BOOST**: Multiplier applied to initial momentum. 1.5x feels natural
- **MOMENTUM_SCROLL_TIMEOUT_MS**: Safety timeout for momentum scrolling (2 seconds recommended)
- **SMOOTH_SCROLL_DIVIDER**: Controls scroll granularity. 3 provides optimal balance

### Usage

1. Enable Mac Mouse Fix style scrolling in your configuration
2. Use trackball movement in scroll layers
3. Movement will continue with momentum after stopping input
4. Momentum strength adapts automatically based on input speed

### Comparison with Standard Scrolling

| Feature       | Standard Mode   | Mac Mouse Fix Style |
| ------------- | --------------- | ------------------- |
| Scroll Feel   | Digital/stepped | Smooth and natural  |
| Momentum      | None            | Yes                 |
| Fine Control  | Low resolution  | High resolution     |
| CPU Usage     | Low             | Slightly higher     |
| Configuration | Simple          | Highly customizable |

### Technical Implementation

- 60 FPS momentum updates for smooth animation
- Floating-point based accumulation for precision
- Timer-based asynchronous processing
- Velocity-based adaptive control

This feature brings the smooth, natural scrolling experience of macOS Magic Mouse and Mac Mouse Fix software to trackball-equipped keyboards.
