Attributes {
    FF_GLOBAL {
        String Server "DODS FreeFrom based on FFND release 4.2.3";
    }
    satellite {
    }
    year {
    }
    day_num {
    }
    hh {
    }
    mm {
    }
    ss {
    }
    DODS_URL {
    }
    DODS_Time {
        String hours_variable GSO_AVHRR.hh;
        String minutes_variable GSO_AVHRR.mm;
        String seconds_variable GSO_AVHRR.ss;
        String gmt_time true;
    }
    DODS_Date {
        String year_variable GSO_AVHRR.year;
        Int32 year_base 1900;
        String year_day_variable GSO_AVHRR.day_num;
    }
}
