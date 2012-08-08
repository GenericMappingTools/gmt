Attributes {
    u {
        String units "meter per second";
        String long_name "Vector wind eastward component";
        String missing_value "-32767";
        String scale_factor "0.005";
    }
    v {
        String units "meter per second";
        String long_name "Vector wind northward component";
        String missing_value "-32767";
        String scale_factor "0.005";
    }
    lat {
        String units "degree North";
    }
    lon {
        String units "degree East";
    }
    time {
        String units "hours from base_time";
    }
    NC_GLOBAL {
        String base_time "88- 10-00:00:00";
        String title " FNOC UV wind components from 1988- 10 to 1988- 13.";
    }
    DODS_EXTRA {
        String Unlimited_Dimension "time_a";
    }
}
