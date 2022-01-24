#ifndef BLE_DIST
#define BLE_DIST

#define TAG "ble_dist"
#define TIMEOUT 120
#define BUF_SIZE 10

#define FCMIN 1e-8
#define BETA 1e-10
#define DCUTOFF 1e-8

# define PATH_LOSS 2.7

class BeaconTracker {

  private:
    std::vector<float> dist_buf;
    std::vector<time_t> time_buf;
    int rssi;
    OneEuroFilter<float, time_t> filter = OneEuroFilter<float, time_t>(1, FCMIN, BETA, DCUTOFF);
    Reading<float, time_t> filt_in;
    Reading<float, time_t> filt_out;

  public:
    std::string name;
    esphome::esp32_ble_tracker::ESPBTUUID uuid;

  public:
    BeaconTracker(std::string n, std::string u) {
      name = n;
      uuid = esphome::esp32_ble_tracker::ESPBTUUID::from_raw(u.c_str());
      dist_buf.reserve(BUF_SIZE+1);
      time_buf.reserve(BUF_SIZE+1);
    }

  private:
    void validate() {
      time_t now = esp_timer_get_time();
      while(!time_buf.empty() && (time_buf.size() > BUF_SIZE || now - time_buf.back() > TIMEOUT * 1e6)) {
        dist_buf.pop_back();
        time_buf.pop_back();
      }
    }

  public:
    void update(int r, int p) {
      filt_in.timestamp = esp_timer_get_time();
      rssi = r;
      dist_buf.insert(dist_buf.begin(), 3.281 * pow(10.0, (p-r)/(10.0 * PATH_LOSS)));
      time_buf.insert(time_buf.begin(), filt_in.timestamp);
      validate();
      auto temp = dist_buf;
      std::sort(temp.begin(), temp.end());
      filt_in.value = dist_buf[std::min((unsigned int)(BUF_SIZE/3), (unsigned)((dist_buf.size()-1)/3))];
      filter.push(&filt_in, &filt_out);
      ESP_LOGD(TAG, "Recognized %s iBeacon: %s", name.c_str(), uuid.to_string().c_str());
      ESP_LOGD(TAG, "  RSSI: %d", r);
      ESP_LOGD(TAG, "  TX Power: %d", p);
      ESP_LOGD(TAG, "  Raw Distance: %g", dist_buf[0]);
      ESP_LOGD(TAG, "  Filtered Distance: %g", filt_out.value);
    }

    float get_rssi() {
      validate();
      return time_buf.empty() ? NAN : (float)rssi;
    }

    float get_dist() {
      validate();
      return time_buf.empty() ? NAN : filt_out.value;
    }

};

BeaconTracker phoneTracker("phone", "D66CFF56-CCB4-47DE-B148-6667181AB156");

void parseAdvertisement(esphome::esp32_ble_tracker::ESPBTDevice dev) {
  if(dev.get_ibeacon().has_value()) {
    auto ib = dev.get_ibeacon().value();
    auto uuid = ib.get_uuid();
    auto txpwr = ib.get_signal_power();
    auto rssi = dev.get_rssi();
    if(uuid == phoneTracker.uuid) {
      phoneTracker.update(rssi, txpwr);
    }
  }
}

#endif
