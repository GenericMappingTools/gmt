Attributes {
    HDF_GLOBAL {
        String Producer_Agency "NASA\\000";
        String Producer_Institution "JPL\\000";
        String Sensor_Name "NSCAT\\000";
        String Project_ID "NSCAT\\000";
        String SIS_ID "697-512-23/1994-08-29\\000";
        String Build_ID "SciSim 3.0\\000";
        String ADEOS_Data_Package_ID "SIMULATED DATA PRODUCT TEST\\000";
        String ADEOS_Data_Package_Type "S\\000";
        String Product_Creation_Time "1995-278T23:30:04.000\\000";
        String Data_Type "L17\\000";
        String Data_Status "COMPLETE\\000";
        Int32 First_Rev_Number 101;
        String First_Rev_Eq_Crossing_Time "1996-215T01:31:07.223\\000";
        Float32 First_Rev_Eq_Crossing_Lon 54.952;
        String First_Data_Time "1996-215T01:13:28.456\\000";
        String Last_Data_Time "1996-215T02:40:22.631\\000";
        Int32 Num_Expected_Output_Records 558;
        Int32 Num_Actual_Output_Records 558;
        String HDF_Build_ID "JPL D-xxxxx 12/15/94\\000";
        String HDF_SIS_ID "JPL D-12059 12/15/94\\000";
        String HDF_Conversion_Organization "JPL PO.DAAC\\000";
        String HDF_Conversion_Time "1996-047T11:13:05       ";
        String Data_Format_Type "HDF\\000";
    }
    WVC_Lat {
        String long_name "latitude";
        String units "deg";
        Float64 scale_factor 0.01;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 22;
        Int16 valid_range -6281, 8051;
    }
    WVC_Lat_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    WVC_Lat_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    WVC_Lon {
        String long_name "longitude";
        String units "deg";
        Float64 scale_factor 0.01;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 22;
        Int16 valid_range 0, -29545;
    }
    WVC_Lon_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    WVC_Lon_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Num_Sigma0 {
        String long_name "The total number of sigma-0 measurements";
        String units "counts";
        Float64 scale_factor 1;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 21;
        Int32 valid_range 0, 21;
    }
    Num_Sigma0_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Num_Sigma0_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Num_Good_Sigma0 {
        String long_name "The number of good sigma-0 measurements";
        String units "counts";
        Float64 scale_factor 1;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 21;
        Int32 valid_range 0, 21;
    }
    Num_Good_Sigma0_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Num_Good_Sigma0_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Num_Beam_12 {
        String long_name "The total number of sigma-0s received from beam 1 or 2";
        String units "counts";
        Float64 scale_factor 1;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 21;
        Int32 valid_range 0, 6;
    }
    Num_Beam_12_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Num_Beam_12_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Num_Beam_34 {
        String long_name "The total number of sigma-0s received from beam 3 or 4";
        String units "counts";
        Float64 scale_factor 1;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 21;
        Int32 valid_range 0, 6;
    }
    Num_Beam_34_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Num_Beam_34_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Num_Beam_56 {
        String long_name "The total number of sigma-0s received from beam 5 or 6";
        String units "counts";
        Float64 scale_factor 1;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 21;
        Int32 valid_range 0, 6;
    }
    Num_Beam_56_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Num_Beam_56_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Num_Beam_78 {
        String long_name "The total number of sigma-0s received from beam 7 or 8";
        String units "counts";
        Float64 scale_factor 1;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 21;
        Int32 valid_range 0, 6;
    }
    Num_Beam_78_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Num_Beam_78_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    WVC_Quality_Flag {
        String long_name "WVC Quality Flag";
        Float64 scale_factor 1;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 21;
        Int32 valid_range 0, 3;
    }
    WVC_Quality_Flag_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    WVC_Quality_Flag_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Cen_Lat {
        String long_name "The center geodetic latitude of a sigma-0 cell";
        String units "deg";
        Float64 scale_factor 0.001;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 24;
        Int32 valid_range -62862, 80490;
    }
    Cen_Lat_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Cen_Lat_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Cen_Lat_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Cen_Lon {
        String long_name "The center longitude of a sigma-0 cell";
        String units "deg";
        Float64 scale_factor 0.001;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 25;
        Int32 valid_range 0, 359997;
    }
    Cen_Lon_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Cen_Lon_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Cen_Lon_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Cell_Azimuth {
        String long_name "The center longitude of a sigma-0 cell";
        String units "deg";
        Float64 scale_factor 0.01;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 23;
        Int16 valid_range 0, -30784;
    }
    Cell_Azimuth_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Cell_Azimuth_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Cell_Azimuth_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Incidence_Angle {
        String long_name "The angle between the local normal vector and cell center direction vector";
        String units "deg";
        Float64 scale_factor 0.01;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 23;
        Int16 valid_range 0, 6259;
    }
    Incidence_Angle_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Incidence_Angle_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Incidence_Angle_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Sigma0 {
        String long_name "The sigma-0 measurement corresponding to each WVC";
        String units "dB";
        Float64 scale_factor 0.01;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 22;
        Int16 valid_range -6069, 344;
    }
    Sigma0_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Sigma0_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Sigma0_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Coeff_A {
        String long_name "The coefficient (alpha) of sigma-0 squared in the sigma-0 variance equation";
        Float64 scale_factor 1e-05;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 23;
        Int16 valid_range 0, 5235;
    }
    Coeff_A_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Coeff_A_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Coeff_A_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Coeff_B {
        String long_name "The coefficient (beta) of sigma-0 in the sigma-0 variance equation";
        Float64 scale_factor 1e-07;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 23;
        Int16 valid_range 0, 793;
    }
    Coeff_B_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Coeff_B_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Coeff_B_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Coeff_C {
        String long_name "The coefficient (gamma) representing the constant term in the sigma-0 variance equation";
        Float64 scale_factor 1e-09;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 23;
        Int16 valid_range 0, 61;
    }
    Coeff_C_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Coeff_C_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Coeff_C_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Sigma0_Quality_Flag {
        String long_name "Bit flags indicating the quality of sigma-0 measurements";
        Int16 valid_range 0, 1;
    }
    Sigma0_Quality_Flag_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Sigma0_Quality_Flag_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Sigma0_Quality_Flag_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    K_Polar {
        String long_name "The polarization index to the model function table";
        Float64 scale_factor 1;
        Float64 scale_factor_err 0;
        Float64 add_offset 0;
        Float64 add_offset_err 0;
        Int32 calibrated_nt 21;
        Int32 valid_range 0, 2;
    }
    K_Polar_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    K_Polar_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    K_Polar_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
    Sigma0_Usable_Flag_1 {
        String long_name "Eight bit flags indicating whether or not the first set of eight sigma-0 measurements is usable for wind retrieval";
        Int32 valid_range 0, 0;
    }
    Sigma0_Usable_Flag_1_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Sigma0_Usable_Flag_1_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Sigma0_Usable_Flag_2 {
        String long_name "Eight bit flags indicating whether or not the second set of eight sigma-0 measurements is usable for wind retrieval";
        Int32 valid_range 0, 0;
    }
    Sigma0_Usable_Flag_2_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Sigma0_Usable_Flag_2_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Sigma0_Usable_Flag_3 {
        String long_name "Eight bit flags indicating whether or not the third set of eight sigma-0 measurements is usable for wind retrieval";
        Int32 valid_range 0, 0;
    }
    Sigma0_Usable_Flag_3_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Sigma0_Usable_Flag_3_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Surface_Flag {
        String long_name "Bit flags indicating land and ice contamination of data";
        Int32 valid_range 0, 0;
    }
    Surface_Flag_dim_0 {
        String name "row";
        String long_name "WVC row index";
    }
    Surface_Flag_dim_1 {
        String name "WVC";
        String long_name "WVC index within a row";
    }
    Surface_Flag_dim_2 {
        String name "Sigma0-Dimension";
        String long_name "Sigma0 position index";
    }
}
