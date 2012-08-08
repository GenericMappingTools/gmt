Attributes {
    Facility {
        String PrincipleInvestigator "Mark Abbott", "Ph.D";
        String DataCenter "COAS Environmental Computer Facility";
        String DrifterType "MetOcean WOCE/OCM";
    }
    Drifter_ID {
        String Description "String value used to uniquely identify each drifter instrument.";
    }
    Date_Sampled {
        String Description "A Date and time string";
        String Timezone "GMT";
    }
    Latitude {
        String Description "Latitude as recorded by ARGOS system";
        String units "degrees_north";
    }
    Longitude {
        String Description "Longitude as recorded by ARGOS system";
        String units "degrees_east";
    }
    SST {
        String Description "Sea Surface Temperature";
        String units "degrees Celsius";
    }
    Ed_490 {
        String Description "Downwelling Irradiance Sensor at 490nm";
        Float32 Wavelength 490;
        String units "microwatts per square cm per nanometer per steradian";
    }
    Lu_412 {
        String Description "Upwelling Radiance Sensor at 412nm";
        Float32 Wavelength 412;
        String units "microwatts per square cm per nanometer per steradian";
    }
    Lu_443 {
        String Description "Upwelling Radiance Sensor at 443nm";
        Float32 Wavelength 443;
        String units "microwatts per square cm per nanometer per steradian";
    }
    Lu_490 {
        String Description "Upwelling Radiance Sensorat at 490nm";
        Float32 Wavelength 490;
        String units "microwatts per square cm per nanometer per steradian";
    }
    Lu_510 {
        String Description "Upwelling Radiance Sensor at 510nm";
        Float32 Wavelength 510;
        String units "microwatts per square cm per nanometer per steradian";
    }
    Lu_555 {
        String Description "Upwelling Radiance Sensor at 555nm";
        Float32 Wavelength 555;
        String units "microwatts per square cm per nanometer per steradian";
    }
    Lu_670 {
        String Description "Upwelling Radiance Sensor at 670nm";
        Float32 Wavelength 670;
        String units "microwatts per square cm per nanometer per steradian";
    }
    Lu_683 {
        String Description "Upwelling Radiance Sensor at 683nm";
        Float32 Wavelength 683;
        String units "microwatts per square cm per nanometer per steradian";
    }
    CHL {
        String Description "Chlorophyl Concentration";
        String units "milligrams per cubic meter";
    }
    FLH {
        String Description "Fluoresence Line Height";
        String units "microwatts per square cm per nanometer per steradian";
    }
    Region {
        String Description "Short description of study area (ex: Oregon, California, Southern Ocean, etc.)";
    }
    Decimal_Day {
        String Description "The time of the mesurement in a decimal format, useful for plots, starts on January 1st 1993 or January 1st 2000.";
    }
    Calibration_File {
        String Description "Path to the file on our file server of the calibration file for the drifter.";
    }
    Drifter_Type {
        String Description "Type of drifter.";
    }
}
