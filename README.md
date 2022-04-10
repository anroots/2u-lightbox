# 2u-lightbox

3d printed, 19'/2U lightbox for your rack.

For NodeMCU microcontroller. Exposes a HTTP API for control and is integratable with Home Assistant.


## HTTP API

## POST /color

Sets RGB color (based on predefined color map). Parameter `color` needs to be int in range of 0...255.

```bash
curl --location --request POST '192.168.0.1/color' \
--form 'color="230"'
```

## POST /brightness

Set the brightness of LEDs. Parameter `brightness` needs to be int in range of 0...255.

```bash
curl --location --request POST '192.168.0.1/brightness' \
--form 'brightness="230"'
```

## POST /state

Switch that toggles the brightness between `0` and `defaultBrightness`. Parameter `state` is either `on` or `off`.

```bash
curl --location --request POST '192.168.0.1/brightness' \
--form 'state="on"'
```

## GET /

Get current state of thje sign (brightness and color) as JSON.


## Home Assistant integration


Add these to `configuration.yaml`:

```yaml
switch:
  - platform: rest
    resource: http://your-hostname.local/state
    method: post
    name: 2u_lightbox_power
    body_on: state=on
    body_off: state=off
    state_resource: http://your-hostname.local/
    is_on_template: |
        {{ value_json.brightness > 0 }}
    headers:
        Content-Type: application/x-www-form-urlencoded
rest_command:
  2u_lightbox_color:
    url: "http://2u-lightbox.atc.ee/color"
    method: post
    content_type: "application/x-www-form-urlencoded"
    payload: "color={{ color }}"
  2u_lightbox_brightness:
    url: "http://2u-lightbox.atc.ee/brightness"
    method: post
    content_type: "application/x-www-form-urlencoded"
    payload: "brightness={{ brightness }}"
```

You'll now be able to turn the sign ON/OFF by calling service "Switch: turn on|off" and selecting the sign as a target;
as well as setting its brightness/color by calling the two services (and passing required parameter):

```yaml
service: rest_command.2u_lightbox_brightness
data:
  brightness: 20
```
