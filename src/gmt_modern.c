/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/

/*
 * Enables new module names without the leading "ps" but only when used
 * in modern mode.  This was done because since in modern mode the default
 * output format is no longer PostScript but PFD and furthermore can be
 * set to any other format (e.g., PNG) as well or in addition to others.
 *
 * Author:	Paul Wessel
 * Date:	03-NOV-2017
 * Version:	6.x
 */

#include "gmt_dev.h"

const char *gmt_current_name (const char *module, char modname[]) {
	/* Given a module, return its document (modern name) and set its classic modname */
	size_t L = strlen (module);

	if (L >= 32U) return module;	/* Safety valve to protect modname array from oversize */

	/* First check for modern names and set the classic name in modname */
	if      (!strncmp (module, "histogram",    9U)) { strcpy (modname, "pshistogram"); return module; }
	else if (!strncmp (module, "colorbar",     8U)) { strcpy (modname, "psscale");     return module; }
	else if (!strncmp (module, "ternary",      7U)) { strcpy (modname, "psternary");   return module; }
	else if (!strncmp (module, "contour",      7U)) { strcpy (modname, "pscontour");   return module; }
	else if (!strncmp (module, "basemap",      7U)) { strcpy (modname, "psbasemap");   return module; }
	else if (!strncmp (module, "events",       6U)) { strcpy (modname, "psevents");    return module; }
	else if (!strncmp (module, "wiggle",       6U)) { strcpy (modname, "pswiggle");    return module; }
	else if (!strncmp (module, "legend",       6U)) { strcpy (modname, "pslegend");    return module; }
	else if (!strncmp (module, "plot3d",       6U)) { strcpy (modname, "psxyz");       return module; }
	else if (!strncmp (module, "segyz",        5U)) { strcpy (modname, "pssegyz");     return module; }
	else if (!strncmp (module, "solar",        5U)) { strcpy (modname, "pssolar");     return module; }
	else if (!strncmp (module, "polar",        5U)) { strcpy (modname, "pspolar");     return module; }
	else if (!strncmp (module, "image",        5U)) { strcpy (modname, "psimage");     return module; }
	else if (!strncmp (module, "coupe",        5U)) { strcpy (modname, "pscoupe");     return module; }
	else if (!strncmp (module, "coast",        5U)) { strcpy (modname, "pscoast");     return module; }
	else if (!strncmp (module, "velo",         4U)) { strcpy (modname, "psvelo");      return module; }
	else if (!strncmp (module, "segy",         4U)) { strcpy (modname, "pssegy");      return module; }
	else if (!strncmp (module, "text",         4U)) { strcpy (modname, "pstext");      return module; }
	else if (!strncmp (module, "plot",         4U)) { strcpy (modname, "psxy");        return module; }
	else if (!strncmp (module, "meca",         4U)) { strcpy (modname, "psmeca");      return module; }
	else if (!strncmp (module, "rose",         4U)) { strcpy (modname, "psrose");      return module; }
	else if (!strncmp (module, "mask",         4U)) { strcpy (modname, "psmask");      return module; }
	else if (!strncmp (module, "clip",         4U)) { strcpy (modname, "psclip");      return module; }
	else if (!strncmp (module, "sac",          3U)) { strcpy (modname, "pssac");       return module; }
	/* Then look for modules that now have a different modern mode name */
	else if (!strncmp (module, "pshistogram", 11U)) { strcpy (modname, module); return "histogram"; }
	else if (!strncmp (module, "psternary",    9U)) { strcpy (modname, module); return "ternary";   }
	else if (!strncmp (module, "pscontour",    9U)) { strcpy (modname, module); return "contour";   }
	else if (!strncmp (module, "psbasemap",    9U)) { strcpy (modname, module); return "basemap";   }
	else if (!strncmp (module, "psevents",     8U)) { strcpy (modname, module); return "events";    }
	else if (!strncmp (module, "pswiggle",     8U)) { strcpy (modname, module); return "wiggle";    }
	else if (!strncmp (module, "pslegend",     8U)) { strcpy (modname, module); return "legend";    }
	else if (!strncmp (module, "pssegyz",      7U)) { strcpy (modname, module); return "segyz";     }
	else if (!strncmp (module, "pssolar",      7U)) { strcpy (modname, module); return "solar";     }
	else if (!strncmp (module, "psscale",      7U)) { strcpy (modname, module); return "colorbar";  }
	else if (!strncmp (module, "pspolar",      7U)) { strcpy (modname, module); return "polar";     }
	else if (!strncmp (module, "psimage",      7U)) { strcpy (modname, module); return "image";     }
	else if (!strncmp (module, "pscoupe",      7U)) { strcpy (modname, module); return "coupe";     }
	else if (!strncmp (module, "pscoast",      7U)) { strcpy (modname, module); return "coast";     }
	else if (!strncmp (module, "psvelo",       6U)) { strcpy (modname, module); return "velo";      }
	else if (!strncmp (module, "pssegy",       6U)) { strcpy (modname, module); return "segy";      }
	else if (!strncmp (module, "pstext",       6U)) { strcpy (modname, module); return "text";      }
	else if (!strncmp (module, "psmeca",       6U)) { strcpy (modname, module); return "meca";      }
	else if (!strncmp (module, "psrose",       6U)) { strcpy (modname, module); return "rose";      }
	else if (!strncmp (module, "psmask",       6U)) { strcpy (modname, module); return "mask";      }
	else if (!strncmp (module, "psclip",       6U)) { strcpy (modname, module); return "clip";      }
	else if (!strncmp (module, "pssac",        5U)) { strcpy (modname, module); return "sac";       }
	else if (!strncmp (module, "psxyz",        5U)) { strcpy (modname, module); return "plot3d";    }
	else if (!strncmp (module, "psxy",         4U)) { strcpy (modname, module); return "plot";      }
	strcpy (modname, module);
	return module;
}

const char *gmt_get_full_name (struct GMTAPI_CTRL *API, const char *module) {
	gmt_M_unused (API);
	/* Given a named module, return its full name if a leading gmt is missing */

	/* Look for classic modules that now have a different modern mode name */
	if      (!strcmp (module, "2kml"))      return "gmt2kml";
	else if (!strcmp (module, "connect"))   return "gmtconnect";
	else if (!strcmp (module, "convert"))   return "gmtconvert";
	else if (!strcmp (module, "defaults"))  return "gmtdefaults";
	else if (!strcmp (module, "get"))       return "gmtget";
	else if (!strcmp (module, "info"))      return "gmtinfo";
	else if (!strcmp (module, "logo"))      return "gmtlogo";
	else if (!strcmp (module, "math"))      return "gmtmath";
	else if (!strcmp (module, "regress"))   return "gmtregress";
	else if (!strcmp (module, "select"))    return "gmtselect";
	else if (!strcmp (module, "set"))       return "gmtset";
	else if (!strcmp (module, "simplify"))  return "gmtsimplify";
	else if (!strcmp (module, "spatial"))   return "gmtspatial";
	else if (!strcmp (module, "vector"))    return "gmtvector";
	else if (!strcmp (module, "which"))     return "gmtwhich";
	else if (!strcmp (module, "pmodeler"))  return "gmtpmodeler";
	else if (!strcmp (module, "flexure"))   return "gmtflexure";
	else if (!strcmp (module, "gravmag3d")) return "gmtgravmag3d";
	return module;
}


const char *gmtlib_get_active_name (struct GMTAPI_CTRL *API, const char *module) {
	/* Given a classic name module, return its name according to the run mode */

	if (!API->GMT->current.setting.use_modern_name)
		return module;
	/* Look for classic modules that now have a different modern mode name */
	if      (!strncmp (module, "pshistogram", 11U)) return "histogram";
	else if (!strncmp (module, "psternary",    9U)) return "ternary";
	else if (!strncmp (module, "pscontour",    9U)) return "contour";
	else if (!strncmp (module, "psbasemap",    9U)) return "basemap";
	else if (!strncmp (module, "psevents",     8U)) return "events";
	else if (!strncmp (module, "pswiggle",     8U)) return "wiggle";
	else if (!strncmp (module, "pslegend",     8U)) return "legend";
	else if (!strncmp (module, "pssegyz",      7U)) return "segyz";
	else if (!strncmp (module, "pssolar",      7U)) return "solar";
	else if (!strncmp (module, "psscale",      7U)) return "colorbar";
	else if (!strncmp (module, "pspolar",      7U)) return "polar";
	else if (!strncmp (module, "psimage",      7U)) return "image";
	else if (!strncmp (module, "pscoupe",      7U)) return "coupe";
	else if (!strncmp (module, "pscoast",      7U)) return "coast";
	else if (!strncmp (module, "psvelo",       6U)) return "velo";
	else if (!strncmp (module, "pssegy",       6U)) return "segy";
	else if (!strncmp (module, "pstext",       6U)) return "text";
	else if (!strncmp (module, "psmeca",       6U)) return "meca";
	else if (!strncmp (module, "psrose",       6U)) return "rose";
	else if (!strncmp (module, "psmask",       6U)) return "mask";
	else if (!strncmp (module, "psclip",       6U)) return "clip";
	else if (!strncmp (module, "pssac",        5U)) return "sac";
	else if (!strncmp (module, "psxyz",        5U)) return "plot3d";
	else if (!strncmp (module, "psxy",         4U)) return "plot";
	return module;
}

bool gmtlib_is_modern_name (struct GMTAPI_CTRL *API, const char *module) {
	bool is_modern = false;	/* If classic */
	gmt_M_unused (API);
	/* Returns true if module is a modern name */

	/* Look for modern mode name modules  */
	if      (!strncmp (module, "histogram", 11U)) is_modern = true;
	else if (!strncmp (module, "ternary",    9U)) is_modern = true;
	else if (!strncmp (module, "contour",    9U)) is_modern = true;
	else if (!strncmp (module, "basemap",    9U)) is_modern = true;
	else if (!strncmp (module, "events",     8U)) is_modern = true;
	else if (!strncmp (module, "wiggle",     8U)) is_modern = true;
	else if (!strncmp (module, "legend",     8U)) is_modern = true;
	else if (!strncmp (module, "segyz",      7U)) is_modern = true;
	else if (!strncmp (module, "solar",      7U)) is_modern = true;
	else if (!strncmp (module, "colorbar",   7U)) is_modern = true;
	else if (!strncmp (module, "polar",      7U)) is_modern = true;
	else if (!strncmp (module, "image",      7U)) is_modern = true;
	else if (!strncmp (module, "coupe",      7U)) is_modern = true;
	else if (!strncmp (module, "coast",      7U)) is_modern = true;
	else if (!strncmp (module, "velo",       6U)) is_modern = true;
	else if (!strncmp (module, "segy",       6U)) is_modern = true;
	else if (!strncmp (module, "text",       6U)) is_modern = true;
	else if (!strncmp (module, "meca",       6U)) is_modern = true;
	else if (!strncmp (module, "rose",       6U)) is_modern = true;
	else if (!strncmp (module, "mask",       6U)) is_modern = true;
	else if (!strncmp (module, "clip",       6U)) is_modern = true;
	else if (!strncmp (module, "plot3d",     5U)) is_modern = true;
	else if (!strncmp (module, "plot",       4U)) is_modern = true;
	else if (!strncmp (module, "sac",        3U)) is_modern = true;
	return is_modern;
}

void gmtlib_set_KOP_strings (struct GMTAPI_CTRL *API) {
	if (API->GMT->current.setting.use_modern_name || API->GMT->current.setting.run_mode == GMT_MODERN) {	/* Must include the required "gmt " prefix */
		API->K_OPT = API->O_OPT = API->P_OPT = "";	/* This are not part of modern mode */
		API->c_OPT = "[-c[<row>,<col>|<index>]] ";	/* -c option for setting next subplot panel */
	}
	else {
		API->K_OPT = "[-K] "; API->O_OPT = "[-O] "; API->P_OPT = "[-P] ";
		API->c_OPT = "";	/* -c is not available in classic mode */
	}
}
