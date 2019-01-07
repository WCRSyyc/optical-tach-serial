# Serial Optical Tachometer

This is a variation of the [Optical Tachometer](https://github.com/WCRSyyc/optical_tachometer) sketch, that uses the serial monitor to display the measurements, instead of an LCD screen.  The minimalist approach, to get something that works, without needing extra hardware.  The LCD screen could be added later.

This also expects the sensor to be connected to the standard ground and 5 volt pins, instead of using pins 3 and 4 for that.

All delay() function calls have been removed from this version.  Any time (versus event triggered) delays are handled by tracking the elapsed time reported by the millis() function.  This demonstrates one of the possible techniques to simplify merging of additional code that should not get blocked just because the tachometer code needs to wait until it has enough data to report.  For example, 2 independent input sensors could be monitored, and each could report results as they become available.  Even if the other sensor is stopped.
