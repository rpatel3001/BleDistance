# BleDistance
Distance tracking for BLE iBeacons in esphome

This code utilizes esphome's esp32_ble_tracker component to track nearby iBeacons. The distance is estimated using the received RSSI and the beacon's advertised 1 meter reference RSSI. The distance is then filtered similarly as in the ESPresense project before being reported.

# Usage
See the example configuration.yaml.
In ble_dist.h, create BeaconTracker objects with the iBeacon UUID for each device you want to track. in the parseAdvertisement function, add a case to the if statement checking the received UUID against each BeaconTracker object and call the update function if it matches. In you configuration.yaml, add a lambda calling the parseAdvertisement function. For each tracker, set up a template sensor that calls the get_dist() function on the appropriate BeaconTracker.

# How it works
When an iBeacon advertisement is received, the distance is calculated based on the received RSSI and the advertised reference power. The calculated distance is stored in a buffer. The buffer is kept to a maximum size defined by BUF_SIZE. Samples older than TIMEOUT seconds are also removed. The 33rd percentile sample of the remaining buffer is then fed into a 1€ filter (modified from https://github.com/haimoz/SoftFilters), with constants taken from ESPresense. The filtered value is reported every 30 seconds, with NAN being reported if there are no samples left in the buffer, due to timeout.
