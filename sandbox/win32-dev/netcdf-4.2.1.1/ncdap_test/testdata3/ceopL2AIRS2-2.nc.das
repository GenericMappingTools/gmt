Attributes {
    longitude {
        String units "degrees_east";
        String standard_name "longitude";
        String axis "X";
        String long_name "longitude";
    }
    latitude {
        String units "degrees_north";
        String standard_name "latitude";
        String axis "Y";
        String long_name "latitude";
    }
    H2OPressureLay {
        String long_name "H2OPressureLay";
        String axis "Z";
        String units "mph";
    }
    time {
        String long_name "time";
        String axis "T";
        String units "days since 1970-01-01";
    }
    H2OMMRStd_Orbit_1735 {
        Float32 _FillValue -1.000000000;
        String unit "gm/kg";
        String H2OPressureLay_ "1100-1000,1000-925,925-850,850-700,700-600,600-500,500-400,400-300,300-250,250-200,200-150,150-100,100-70,70-50";
        String long_name "H2OMMRStd_Orbit_1735";
        String GCTP_Projection_Name "Geographic";
        String GCTP_Spheroid_Name "Clarke 1866";
    }
    H2OMMRStd_Orbit_1742 {
        Float32 _FillValue -1.000000715;
        String unit "gm/kg";
        String H2OPressureLay_ "1100-1000,1000-925,925-850,850-700,700-600,600-500,500-400,400-300,300-250,250-200,200-150,150-100,100-70,70-50";
        String long_name "H2OMMRStd_Orbit_1742";
        String GCTP_Projection_Name "Geographic";
        String GCTP_Spheroid_Name "Clarke 1866";
    }
    NC_GLOBAL {
        String Conventions "CF-1.0";
        String Institution "CSISS,GMU";
    }
}
