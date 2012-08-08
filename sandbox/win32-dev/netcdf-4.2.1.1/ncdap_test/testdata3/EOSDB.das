Attributes {
    Facility {
        String PrincipleInvestigator "Mark Abbott";
        String DataCenter "COAS Environmental Computer Facility";
    }
    LON {
        String units "degrees_east";
        String modulo " ";
        String point_spacing "even";
    }
    LAT {
        String units "degrees_north";
        String point_spacing "even";
    }
    TIME {
        String units "days since 1700-01-01 00:00:00";
        String time_origin "1-JAN-1700";
        String point_spacing "uneven";
        String edges "TIMEedges";
    }
    TIMEedges {
        String edges " ";
    }
    SST {
        Float64 missing_value -9.9999998e+33;
        Float64 _FillValue -9.9999998e+33;
        String long_name "Sea Surface Temperature";
        String history "From all_coads_grid";
        String units "deg C";
    }
    NC_GLOBAL {
        String history "FERRET V4.11 (debug/no GUI) 19-Nov-95FERRET V4.20 (debug/no GUI) 12-Mar-96";
        String title "COADS Surface Marine Observations (1854-1993)";
    }
}
