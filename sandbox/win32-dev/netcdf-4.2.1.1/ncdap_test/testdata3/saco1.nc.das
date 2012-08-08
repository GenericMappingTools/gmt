Attributes {
    lat {
        String longname "latitude";
        String units "degrees_north";
    }
    lon {
        String longname "longitude";
        Float32 modulo 360.0000000;
        String units "degrees_east";
    }
    height {
        String longname "gage height";
        String units "meters";
        String positive "up"; 
        String C_format "%.2f";
        Float64 minimum 2.4100000000000001;
        Float64 maximum 3.7400000000000002;
    }
    time {
        String longname "time";
        String units "days since Sun Aug  6 00:15:00 2000";
        String C_format "%.6f";
    }
    river_discharge {
        String longname "River Discharge";
        String units "cubic feet per second";
        Float64 minimum 789.00000000000000;
        Float64 maximum 1780.0000000000000;
    }
    NC_GLOBAL {
        String Conventions "GDT 1.3";
        String history "Created Thu Aug 17 11:10:11 2000 by jevans";
        String institution "School of Marine Sciences, University of Maine";
        String production "gage height observation";
        Float64 starting_julian_day_number 2451762.6770833335;
        String starting_julian_day_str "Sun Aug  6 04:15:00 2000";
        Float64 ending_julian_day_number 2451771.0520833335;
        String ending_julian_day_str "Mon Aug 14 13:15:00 2000";
        String time_zone "UTC";
    }
}
