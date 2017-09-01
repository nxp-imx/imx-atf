/* Copyright 2017 NXP */
/* Include file detailing the resource partitioning for ATF */

/* resources that are going to stay in secure partition */
sc_rsrc_t secure_rsrcs[] = {
	SC_R_MU_1A,
	SC_R_A35,
	SC_R_A35_0,
	SC_R_A35_1,
	SC_R_A35_2,
	SC_R_A35_3,
	SC_R_GIC,
	SC_R_SYSTEM,
};

/* resources that have register access for non-secure domain */
sc_rsrc_t ns_access_allowed[] = {
	SC_R_GIC,
};
