Attributes {
    Facility {
        String PrincipleInvestigator "Mark Abbott", "Ph.D";
        String DataCenter "COAS Environmental Computer Facility";
        String DrifterType "MetOcean WOCE/OCM";
    }
    Drifters {
        battery {
            String Description "Battery Voltage (VBAT)";
            String units "volts";
        }
        checksum {
            String Description "Low 8 bits of the sum of the specified bytes in ARGOS transmission. (Checksum)";
            String units "unknown";
        }
        data_age {
            String Description "Optical Sensor Data Age: Elapsed time between sample time and time of delivery to ARGOS system. (DA)";
            String units "minutes";
        }
        ed490 {
            String Description "Downwelling Irradiance Sensor at 490nm";
            Float32 Wavelength 490;
            String units "microwatts per square cm per nanometer per steradian";
        }
        ed490_stdev {
            String Description "Standard deviation of Downwelling Irradiance Sensor at 490nm";
            Float32 Wavelength 490;
            String units "counts";
        }
        instrument_id {
            String Description "String value used to uniquely identify each drifter instrument.";
        }
        flag_argos {
        }
        flag_gps {
        }
        flag_optical {
        }
        flag_sampling {
        }
        flag_tracking {
        }
        gpshour {
            String Description "GPS Acquisition Time: Hour of day of GPS fix. (GPSHour)";
            String units "hour";
        }
        gpslat {
            String Description "GPS Latitiude (Lat)";
            String units "degrees_north";
        }
        gpslon {
            String Description "GPS Longitude (Lon)";
            String units "degrees_east";
        }
        latitude {
            String Description "Latitude as recorded by ARGOS system";
            String units "degrees_north";
        }
        location {
            String Description "String describing general location (southern ocean,oregon coast, etc.) of drifter deployment.";
        }
        longitude {
            String Description "Longitude as recorded by ARGOS system";
            String units "degrees_east";
        }
        lu412 {
            String Description "Upwelling Radiance Sensor at 412nm";
            Float32 Wavelength 412;
            String units "microwatts per square cm per nanometer per steradian";
        }
        lu412_stdev {
            String Description "Standard deviation of Upwelling Radiance Sensor at 412nm";
            Float32 Wavelength 412;
            String units "counts";
        }
        lu443 {
            String Description "Upwelling Radiance Sensor at 443nm";
            Float32 Wavelength 443;
            String units "microwatts per square cm per nanometer per steradian";
        }
        lu443_stdev {
            String Description "Standard deviation of Upwelling Radiance Sensor at 443nm";
            Float32 Wavelength 443;
            String units "counts";
        }
        lu490 {
            String Description "Upwelling Radiance Sensorat at 490nm";
            Float32 Wavelength 490;
            String units "microwatts per square cm per nanometer per steradian";
        }
        lu490_stdev {
            String Description "Standard deviation of Upwelling Radiance Sensor at 490nm";
            Float32 Wavelength 490;
            String units "counts";
        }
        lu510 {
            String Description "Upwelling Radiance Sensor at 510nm";
            Float32 Wavelength 510;
            String units "microwatts per square cm per nanometer per steradian";
        }
        lu510_stdev {
            String Description "Standard deviation of Upwelling Radiance Sensor at 510nm";
            Float32 Wavelength 510;
            String units "counts";
        }
        lu555 {
            String Description "Upwelling Radiance Sensor at 555nm";
            Float32 Wavelength 555;
            String units "microwatts per square cm per nanometer per steradian";
        }
        lu555_stdev {
            String Description "Standard deviation of Upwelling Radiance Sensor at 555nm";
            Float32 Wavelength 555;
            String units "counts";
        }
        lu670 {
            String Description "Upwelling Radiance Sensor at 670nm";
            Float32 Wavelength 670;
            String units "microwatts per square cm per nanometer per steradian";
        }
        lu670_stdev {
            String Description "Standard deviation of Upwelling Radiance Sensor at 670nm";
            Float32 Wavelength 670;
            String units "counts";
        }
        lu683 {
            String Description "Upwelling Radiance Sensor at 683nm";
            Float32 Wavelength 683;
            String units "microwatts per square cm per nanometer per steradian";
        }
        lu683_stdev {
            String Description "Standard deviation of Upwelling Radiance Sensor at 683nm";
            Float32 Wavelength 683;
            String units "counts";
        }
        max_depth {
            String Description "Maximum Depth Voltage reading from the previous 10.5 minute period. (MDV)";
            String units "volts";
        }
        night_len {
            String Description "Length of the previous night (LNL)";
            String units "minutes";
        }
        nsamp {
            String Description "Message ID: Always set to 3 for GPS data Message";
            String units "unknown";
        }
        numaves {
            String Description "Number of samples in OCM data (NAve)";
            String units "unknown";
        }
        press_volts {
            String Description "Pressure sensor voltage at surface as determined by the surface check function. (SV)";
            String units "volts";
        }
        rellat1hr {
            String Description "Relative Latitude: Change in latitude in the last hour as determined by the GPS subsystem (Rlat1)";
            String units "degrees_north";
        }
        rellat2hr {
            String Description "Relative Latitude: Change in latitude in the last 2 hours as determined by the GPS subsystem (Rlat2)";
            String units "degrees_north";
        }
        rellon1hr {
            String Description "Relative Longitude: Change in longitude in the last hour as determined by the GPS subsystem (Rlon1)";
            String units "degrees_east";
        }
        rellon2hr {
            String Description "Relative Longitude: Change in longitude in the last 2 hours as determined by the GPS subsystem (Rlon2)";
            String units "degrees_east";
        }
        sst {
            String Description "Sea Surface Temperature (SST)";
            String units "Degrees C";
        }
        sub_wait {
            String Description "Average time that bouy software waited for bouy to come to surface before measuring irradiance. (ASW)";
            String units "minutes";
        }
        surf_volts {
            String Description "Averaged amplified pressure sensor voltage as measured during the surface check function. (AV)";
            String units "volts";
        }
        surface_average {
            String Description "Average duration of time that the bouy remained on the surface during the surface check function. (AST)";
            String units "minutes";
        }
        surface_percent {
            String Description "Percentage of time that the bouy was on the surface during the surface check function. (PTOS)";
            String units "percentage";
        }
        time {
            String Description "Time data sample was received by ARGOS system";
            String units "Decimal days since Jan 1, 1993 (01/01/1993)";
        }
        ttff {
            String Description "Time between GPS power on and first fix: Time To First Fix (TTFF)";
            String units "Decimal days since Jan 1, 1993 (01/01/1993)";
        }
        maxgps {
            String Description "Strongest GPS Signal Level (dB0)";
            String units "dB";
        }
        rellat3hr {
            String Description "Relative Latitude: Change in latitude in the last 3 hours as determined by the GPS subsystem (Rlat3)";
            String units "degrees_north";
        }
        rellon3hr {
            String Description "Relative Longitude: Change in longitude in the last 3 hours as determined by the GPS subsystem (Rlon3)";
            String units "degrees_east";
        }
    }
}
