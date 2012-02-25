/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 string types.

   $Id$
*/

#include <config.h>
#include <nc_tests.h>

#define FILE_NAME "tst_strings.nc"
#define DIM_LEN 9
#define ATT_NAME "measure_for_measure_att"
#define DIM_NAME "line"
#define VAR_NAME "measure_for_measure_var"
#define NDIMS 1

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 string type.\n");
   printf("*** testing string attribute...");
   {
#define ATT_LEN_1 1
#define MOUNTAIN_RANGE "mountain_range"
      size_t att_len;
      int ndims, nvars, natts, unlimdimid;
      nc_type att_type;
      int ncid, i;
      char *data_in[ATT_LEN_1] = {NULL};
      char *data[ATT_LEN_1] = {"R"};
   
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, MOUNTAIN_RANGE, NC_STRING, ATT_LEN_1, data)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, MOUNTAIN_RANGE, &att_type, &att_len)) ERR;
      if (att_type != NC_STRING || att_len != ATT_LEN_1) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, MOUNTAIN_RANGE, &att_type, &att_len)) ERR;
      if (att_type != NC_STRING || att_len != ATT_LEN_1) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, MOUNTAIN_RANGE, data_in)) ERR;
      for (i = 0; i < att_len; i++)
      	 if (strcmp(data_in[i], data[i])) ERR;
      if (nc_free_string(ATT_LEN_1, (char **)data_in)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing string attribute...");
   {
#define ATT_LEN 9
      size_t att_len;
      int ndims, nvars, natts, unlimdimid;
      nc_type att_type;
      int ncid, i;
      char *data_in[ATT_LEN];
      char *data[ATT_LEN] = {"Let but your honour know",
			     "Whom I believe to be most strait in virtue",
			     "That, in the working of your own affections",
			     "Had time cohered with place or place with wishing",
			     "Or that the resolute acting of your blood",
			     "Could have attain'd the effect of your own purpose",
			     "Whether you had not sometime in your life",
			     "Err'd in this point which now you censure him",
			     "And pull'd the law upon you."};
   
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, NC_STRING, ATT_LEN, data)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &att_type, &att_len)) ERR;
      if (att_type != NC_STRING || att_len != ATT_LEN) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &att_type, &att_len)) ERR;
      if (att_type != NC_STRING || att_len != ATT_LEN) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, ATT_NAME, data_in)) ERR;
      for (i = 0; i < att_len; i++)
      	 if (strcmp(data_in[i], data[i])) ERR;
      if (nc_free_string(att_len, (char **)data_in)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing string var functions...");
   {
#define MOBY_LEN 16
      int ncid, varid, dimids[NDIMS];
      char *data[] = {"Perhaps a very little thought will now enable you to account for ",
		      "those repeated whaling disasters--some few of which are casually ",
		      "chronicled--of this man or that man being taken out of the boat by ",
		      "the line, and lost.",
		      "For, when the line is darting out, to be seated then in the boat, ",
		      "is like being seated in the midst of the manifold whizzings of a ",
		      "steam-engine in full play, when every flying beam, and shaft, and wheel, ",
		      "is grazing you.",
		      "It is worse; for you cannot sit motionless in the heart of these perils, ",
		      "because the boat is rocking like a cradle, and you are pitched one way and ",
		      "the other, without the slightest warning;",
		      "But why say more?",
		      "All men live enveloped in whale-lines.",
		      "All are born with halters round their necks; but it is only when caught ",
		      "in the swift, sudden turn of death, that mortals realize the silent, subtle, ",
		      "ever-present perils of life."};
      char *data_in[MOBY_LEN];
      int i;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, MOBY_LEN, dimids)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_STRING, NDIMS, dimids, &varid)) ERR;
      if (nc_put_var_string(ncid, varid, (const char **)data)) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
     if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
     if (nc_get_var_string(ncid, varid, data_in)) ERR;
     for (i=0; i<MOBY_LEN; i++)
	if (strcmp(data_in[i], data[i])) ERR;
     if (nc_free_string(MOBY_LEN, (char **)data_in)) ERR;
     if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing string attributes...");
   {
#define SOME_PRES 16
#define NDIMS_PRES 1
#define ATT2_NAME "presidents"

      int ncid, i;
      char *data[SOME_PRES] = {"Washington", "Adams", "Jefferson", "Madison",
			       "Monroe", "Adams", "Jackson", "VanBuren",
			       "Harrison", "Tyler", "Polk", "Tayor",
			       "Fillmore", "Peirce", "Buchanan", "Lincoln"};
      char *data_in[SOME_PRES];

      /* Create a file with string attribute. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_put_att_string(ncid, NC_GLOBAL, ATT2_NAME, SOME_PRES, (const char **)data)) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_get_att_string(ncid, NC_GLOBAL, ATT2_NAME, (char **)data_in)) ERR;
      for (i=0; i < SOME_PRES; i++)
	 if (strcmp(data_in[i], data[i])) ERR;
      
      /* Must free your data! */
      if (nc_free_string(SOME_PRES, (char **)data_in)) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing string fill value...");
   {
#define NUM_PRES 43
#define SOME_PRES 16
#define NDIMS_PRES 1
#define VAR_NAME_P "presidents"
      int ncid, varid, i, dimids[NDIMS_PRES];
      size_t start[NDIMS_PRES], count[NDIMS_PRES];
      char *data[SOME_PRES] = {"Washington", "Adams", "Jefferson", "Madison",
			       "Monroe", "Adams", "Jackson", "VanBuren",
			       "Harrison", "Tyler", "Polk", "Tayor",
			       "Fillmore", "Peirce", "Buchanan", "Lincoln"};
      char *data_in[NUM_PRES];

      /* Create a file with NUM_PRES strings, and write SOME_PRES of
       * them. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, NUM_PRES, dimids)) ERR;
      if (nc_def_var(ncid, VAR_NAME_P, NC_STRING, NDIMS_PRES, dimids, &varid)) ERR;
      start[0] = 0;
      count[0] = SOME_PRES;
      if (nc_put_vara_string(ncid, varid, start, count, (const char **)data)) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_get_var_string(ncid, varid, data_in)) ERR;
      for (i = 0; i < NUM_PRES; i++)
      {
	 if (i < SOME_PRES && strcmp(data_in[i], data[i])) ERR;
	 if (i >= SOME_PRES && strcmp(data_in[i], "")) ERR;
      }
      
      /* Must free your data! */
      if (nc_free_string(NUM_PRES, (char **)data_in)) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing long string variable...");
   {
#define VAR_NAME2 "empty"
#define ATT_NAME2 "empty"
#define DHR_LEN 30
#define DIM_NAME1 "article"
#define VAR_NAME1 "universal_declaration_of_human_rights"
      int var_dimids[NDIMS];
      int ndims, nvars, natts, unlimdimid;
      nc_type var_type;
      char var_name[NC_MAX_NAME + 1];
      int var_natts, var_ndims;
      int ncid, varid, i, dimids[NDIMS], varid2;
      char *data_in[DHR_LEN];
      char *data[DHR_LEN] = {
	 "All human beings are born free and equal in dignity and rights. "
	 "They are endowed with reason and "
	 "conscience and should act towards one another in a "
	 "spirit of brotherhood.",
	 "Everyone is entitled to all the rights and freedoms set "
	 "forth in this Declaration, without distinction of "
	 "any kind, such as race, colour, sex, language, religion, "
	 "political or other opinion, national or social "
	 "origin, property, birth or other status. Furthermore, no "
	 "distinction shall be made on the basis of the "
	 "political, jurisdictional or international status of the "
	 "country or territory to which a person belongs, "
	 "whether it be independent, trust, non-self-governing or "
	 "under any other limitation of sovereignty.",
	 "Everyone has the right to life, liberty and security of person.",
	 "No one shall be held in slavery or servitude; slavery and the "
	 "slave trade shall be prohibited in all their forms.",
	 "No one shall be subjected to torture or to cruel, "
	 "inhuman or degrading treatment or punishment.",
	 "Everyone has the right to recognition everywhere as "
	 "a person before the law.",
	 "All are equal before the law and are entitled without "
	 "any discrimination to equal protection of the law. All are "
	 "entitled to equal protection against any discrimination in "
	 "violation of this Declaration and against any incitement "
	 "to such discrimination.",
	 "Everyone has the right to an effective remedy by the "
	 "competent national tribunals for acts violating the "
	 "fundamental rights granted "
	 "him by the constitution or by law.",
	 "No one shall be subjected to arbitrary arrest, detention or exile.",
	 "Everyone is entitled in full equality to a fair and public "
	 "hearing by an independent and impartial tribunal, in the "
	 "determination of his rights and obligations and of any "
	 "criminal charge against him.",
	 "(1) Everyone charged with a penal offence has the right "
	 "to be presumed innocent until proved guilty according to law in a "
	 "public trial at which he has had all the guarantees "
	 "necessary for his defence."
	 "(2) No one shall be held guilty of any penal offence "
	 "on account of any act or omission which did not constitute a penal "
	 "offence, under national or international law, at the time "
	 "when it was committed. Nor shall a heavier penalty be imposed "
	 "than the one that was applicable at the time the penal "
	 "offence was committed.",
	 "No one shall be subjected to arbitrary interference with "
	 "his privacy, family, home or correspondence, nor to attacks upon "
	 "his honour and reputation. Everyone has the right to the "
	 "protection of the law against such interference or attacks.",
	 "(1) Everyone has the right to freedom of movement and "
	 "residence within the borders of each state."
	 "(2) Everyone has the right to leave any country, "
	 "including his own, and to return to his country.",
	 "(1) Everyone has the right to seek and to enjoy in "
	 "other countries asylum from persecution. "
	 "(2) This right may not be invoked in the case of prosecutions "
	 "genuinely arising from non-political crimes or from acts "
	 "contrary to the purposes and principles of the United Nations.",
	 "(1) Everyone has the right to a nationality. (2) No one shall "
	 "be arbitrarily deprived of his nationality nor denied the "
	 "right to change his nationality.",
	 "(1) Men and women of full age, without any limitation "
	 "due to race, nationality or religion, have the right "
	 "to marry and to found a family. "
	 "They are entitled to equal rights as to marriage, "
	 "during marriage and at its dissolution. (2) Marriage "
	 "shall be entered into only with the free and full "
	 "consent of the intending spouses. (3) The family is "
	 "the natural and fundamental group unit of society and "
	 "is entitled to protection by society and the State.",
	 "(1) Everyone has the right to own property alone as "
	 "well as in association with others. (2) No one shall "
	 "be arbitrarily deprived of his property.",
	 "Everyone has the right to freedom of thought, conscience "
	 "and religion; this right includes freedom to change "
	 "his religion or belief, and freedom, "
	 "either alone or in community with others and in "
	 "public or private, to manifest his religion or "
	 "belief in teaching, practice, worship and observance.",
	 "Everyone has the right to freedom of opinion and "
	 "expression; this right includes freedom to hold "
	 "opinions without interference and to seek, receive "
	 "and impart information and ideas through any media "
	 "and regardless of frontiers.",
	 "(1) Everyone has the right to freedom of peaceful assembly "
	 "and association. (2) No one may be compelled to belong to "
	 "an association.",
	 "(1) Everyone has the right to take part in the government "
	 "of his country, directly or through freely chosen representatives. "
	 "(2) Everyone has the right of equal access to public "
	 "service in his country. (3) The will of the people "
	 "shall be the basis of the authority of government; "
	 "this will shall be expressed in periodic and genuine "
	 "elections which shall be by universal and equal suffrage "
	 "and shall be held by secret vote or by "
	 "equivalent free voting procedures.",
	 "Everyone, as a member of society, has the right to "
	 "social security and is entitled to realization, "
	 "through national effort and international co-operation "
	 "and in accordance with the organization and resources of "
	 "each State, of the economic, social and cultural rights "
	 "indispensable for his dignity and the free "
	 "development of his personality.",
	 "(1) Everyone has the right to work, to free choice "
	 "of employment, to just and favourable conditions of "
	 "work and to protection against unemployment. "
	 "(2) Everyone, without any discrimination, has the right "
	 "to equal pay for equal work. (3) Everyone who works "
	 "has the right to just and favourable "
	 "remuneration ensuring for himself and his family an "
	 "existence worthy of human dignity, and supplemented, "
	 "if necessary, by other means of social protection."
	 "(4) Everyone has the right to form and to join trade "
	 "unions for the protection of his interests.",
	 "Everyone has the right to rest and leisure, including "
	 "reasonable limitation of working hours and periodic "
	 "holidays with pay.",
	 "(1) Everyone has the right to a standard of living "
	 "adequate for the health and well-being of himself "
	 "and of his family, including food, clothing, housing "
	 "and medical care and necessary social services, and "
	 "the right to security in the event of unemployment, "
	 "sickness, disability, widowhood, old age or other lack "
	 "of livelihood in circumstances beyond his control. "
	 "(2) Motherhood and childhood are entitled to special "
	 "care and assistance. All children, whether "
	 "born in or out of wedlock, shall enjoy the same "
	 "social protection.",
	 "(1) Everyone has the right to education. Education "
	 "shall be free, at least in the elementary and "
	 "fundamental stages. Elementary education shall be compulsory. "
	 "Technical and professional education shall be made generally "
	 "available and higher education "
	 "shall be equally accessible to all on the basis of merit. "
	 "(2) Education shall be directed to the full development of "
	 "the human personality and to the strengthening of respect for "
	 "human rights and fundamental freedoms. "
	 "It shall promote understanding, tolerance and friendship among "
	 "all nations, racial or religious groups, and shall further the "
	 "activities of the United Nations "
	 "for the maintenance of peace. (3) Parents have a prior right to "
	 "choose the kind of education that shall be given to their children.",
	 "(1) Everyone has the right freely to participate in the cultural "
	 "life of the community, to enjoy the arts and to share in scientific "
	 "advancement and its benefits. "
	 "(2) Everyone has the right to the protection of the moral "
	 "and material interests resulting from any scientific, "
	 "literary or artistic production of which he is the author.",
	 "Everyone is entitled to a social and international order in "
	 "which the rights and freedoms set forth in this Declaration "
	 "can be fully realized.",
	 "(1) Everyone has duties to the community in which alone "
	 "the free and full development of his personality is possible. "
	 "(2) In the exercise of his rights and "
	 "freedoms, everyone shall be subject only to such limitations "
	 "as are determined by law solely for the purpose "
	 "of securing due recognition and respect for the rights "
	 "and freedoms of others and of meeting the just requirements "
	 "of morality, public order and the general welfare in a "
	 "democratic society. (3) These rights and freedoms may in no "
	 "case be exercised contrary to the purposes and "
	 "principles of the United Nations.",
	 "Nothing in this Declaration may be interpreted as implying "
	 "for any State, group or person any right to engage in any "
	 "activity or to perform any act aimed at the destruction of "
	 "any of the rights and freedoms set forth herein."
      };
      char *empty_string[] = {""};
   
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Create an array of strings for the Universal Declaraion of Human Rights. */
      if (nc_def_dim(ncid, DIM_NAME1, DHR_LEN, dimids)) ERR;
      if (nc_def_var(ncid, VAR_NAME1, NC_STRING, NDIMS, dimids, &varid)) ERR;

      /* Create a scalar variable for the empty string. */
      if (nc_def_var(ncid, VAR_NAME2, NC_STRING, 0, NULL, &varid2)) ERR;

      /* Check some stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, varid, var_name, &var_type, &var_ndims,
      		     var_dimids, &var_natts)) ERR;
      if (var_type != NC_STRING || strcmp(var_name, VAR_NAME1) || var_ndims != NDIMS ||
      	  var_dimids[0] != dimids[0]) ERR;

      /* Write the universal declaraion of human rights. */
      if (nc_put_var(ncid, varid, data)) ERR;

      /* Write an empty string with an empty attribute. */
      if (nc_put_var(ncid, varid2, empty_string)) ERR;
      if (nc_put_att(ncid, varid2, ATT_NAME2, NC_STRING, 0, empty_string)) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
      
      /* Check declaration. */
      if (nc_inq_varid(ncid, VAR_NAME1, &varid)) ERR;
      if (nc_inq_var(ncid, varid, var_name, &var_type, &var_ndims,
		     var_dimids, &var_natts)) ERR;
      if (var_type != NC_STRING || strcmp(var_name, VAR_NAME1) || var_ndims != NDIMS ||
	  var_dimids[0] != dimids[0]) ERR;
      if (nc_get_var(ncid, varid, data_in)) ERR;
      for (i = 0; i < DHR_LEN; i++)
	 if (strcmp(data_in[i], data[i])) ERR;
      if (nc_free_string(DHR_LEN, data_in));

      /* Check the empty var and att. */
      if (nc_inq_varid(ncid, VAR_NAME2, &varid)) ERR;
      if (nc_get_var(ncid, varid, data_in)) ERR;
      if (strcmp(data_in[0], empty_string[0])) ERR;
      if (nc_free_string(1, data_in)) ERR;
      if (nc_get_att(ncid, varid, ATT_NAME2, NULL)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

