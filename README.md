# BleDistance
Distance tracking for BLE iBeacons in esphome

![Example screenshot](/screenshot.png)

This code utilizes esphome's esp32_ble_tracker component to track nearby iBeacons. The distance is estimated using the received RSSI and the beacon's advertised 1 meter reference RSSI. The distance is then passed through an adaptive low pass filter before being reported. This works on my personal boards and setup, YMMV.

# Usage
See the example configuration.yaml.
Add new BeaconTracker objects in a lambda under the esphome: on_boot: section of your configuration.yaml, with an iBeacon UUID and unique name for each device you want to track. Add a lambda calling the parseAdvertisement function in esp32_ble_tracker: on_ble_advertise:. For each tracker, set up a template sensor that gets a tracker using getTracker() and then calls get_dist() on the returned object. The 1€ filter fcmin and beta parameters are optionally adjustable when creating a tracker. If a sensor is too jittery when stationary, decrease fcmin. If the lag when moving is unacceptable, increase beta.

# How it works
When an iBeacon advertisement is received, the distance is calculated based on the received RSSI and the advertised reference power. The calculated distance is stored in a buffer. The buffer is kept to a maximum size defined by `BUF_SIZE`. Samples older than `TIMEOUT` seconds are also removed. The 33rd percentile sample of the remaining buffer is then fed into a 1€ filter (adapted from the C++ example by Jonathan Aceitune [here](http://cristal.univ-lille.fr/~casiez/1euro/) and fixed according to [this issue](https://github.com/haimoz/SoftFilters/issues/1)). By default, the reported value if there are no samples remaining in the buffer is `NAN`, which causes the Home Assistant sensor to go unavailable.
