#ifndef ONEEURO_H
#define ONEEURO_H

struct low_pass_filter {
	low_pass_filter() : hatxprev(0), hadprev(false) {}
	float operator()(float x, float alpha) {
		float hatx;
		if(hadprev) {
			hatx = alpha * x + (1-alpha) * hatxprev;
		} else {
			hatx = x;
			hadprev = true;
		}
		hatxprev = hatx;
		return hatx;
	}
	float hatxprev;
	bool hadprev;
};

struct OneEuro {
	OneEuro(float _freq, float _mincutoff, float _beta, float _dcutoff) : mincutoff(_mincutoff), beta(_beta), dcutoff(_dcutoff), last_time_(-1) {}
	float operator()(float x, time_t t = -1) {
		float dx = 0.0;
		
		if(last_time_ != -1 && t != -1 && t != last_time_) {
			dt = (t - last_time_) / 1e6;
		}
		last_time_ = t;
		
		if(xfilt_.hadprev)
			dx = (x - xfilt_.hatxprev) / dt;
		
		float edx = dxfilt_(dx, alpha(dcutoff));
		float cutoff = mincutoff + beta * abs(edx);
		return xfilt_(x, alpha(cutoff));
	}
	
	float mincutoff, beta, dcutoff;
private:
	float alpha(float cutoff) {
		float tau = 1.0 / (2 * M_PI * cutoff);
		return 1.0 / (1.0 + tau / dt);
	}
	
	time_t last_time_;
	float dt;
	low_pass_filter xfilt_, dxfilt_;
};

#endif
