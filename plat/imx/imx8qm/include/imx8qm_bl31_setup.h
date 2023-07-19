#ifndef IMX8QM_BL31_SETUP_H
#define IMX8QM_BL31_SETUP_H

#if defined(SPD_trusty)
int configure_memory_region_owned_by_os_part(int vpu_part);
int get_partition_number(int* os_part, int* dpu_part);
#endif
#endif

