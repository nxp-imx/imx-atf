/* Copyright 2017 NXP */
/* Include file detailing the resource partitioning for ATF */

/* resources that are going to stay in secure partition */
sc_rsrc_t secure_rsrcs[] = {
  SC_R_MU_1A,
  SC_R_A53,
  SC_R_A53_0,
  SC_R_A53_1,
  SC_R_A53_2,
  SC_R_A53_3,
  SC_R_A72,
  SC_R_A72_0,
  SC_R_A72_1,
  SC_R_GIC,
  SC_R_GIC_SMMU,
  SC_R_CCI
};

/* resources that have register access for non-secure domain */
sc_rsrc_t ns_access_allowed[] = {
  SC_R_GIC,
  SC_R_GIC_SMMU,
  SC_R_CCI
};

struct mem_region {
	unsigned long start;
	unsigned long end;
};

struct mem_region ns_mem_region[] = {
	{0x000000000, 0x01BFFFFFF},
	{0x034000000, 0x037FFFFFF},
	{0x038000000, 0x03BFFFFFF},
	{0x060000000, 0x06FFFFFFF},
	{0x070000000, 0x07FFFFFFF},
	{0x080000000, 0x0FFFFFFFF},
	{0x400000000, 0x43FFFFFFF},
	{0x880000000, 0xFFFFFFFFF},
};
