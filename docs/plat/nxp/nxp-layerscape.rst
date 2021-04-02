Overview
========

The Layerscape SoCs supported on TF-A are

* `LX2160ARDB`_
* `LS1046ARDB`_

Platforms and Boot Sources Supported
------------------------------------

+------------+-----+------+-----+------+------+-------------+--------------+----------------+
| PLAT       | SD  | QSPI | NOR | NAND | eMMC | FlexSPI-NOR | FlexSPI-NAND | FIP-DDR Needed |
+============+=====+======+=====+======+======+=============+==============+================+
| LX2160ARDB | yes |      |     |      | yes  | yes         |              | yes            |
+------------+-----+------+-----+------+------+-------------+--------------+----------------+
| LS1046ARDB | yes | yes  |     |      |      |             |              | no             |
+------------+-----+------+-----+------+------+-------------+--------------+----------------+

Boot Sequence
-------------
::

+                           Secure World        |     Normal World
+ EL0                                           |
+                                               |
+ EL1                           BL32(Tee OS)    |     kernel
+                                ^ |            |       ^
+                                | |            |       |
+ EL2                            | |            |     BL33(u-boot)
+                                | |            |      ^
+                                | v            |     /
+ EL3        BootROM --> BL2 --> BL31 ---------------/
+

Boot Sequence with FIP-DDR
--------------------------
::

+                           Secure World        |     Normal World
+ EL0                                           |
+                                               |
+ EL1               fip-ddr     BL32(Tee OS)    |     kernel
+                     ^ |         ^ |           |       ^
+                     | |         | |           |       |
+ EL2                 | |         | |           |     BL33(u-boot)
+                     | |         | |           |      ^
+                     | v         | v           |     /
+ EL3     BootROM --> BL2 -----> BL31 ---------------/
+

How to Build
============

Code Locations
--------------

* OP-TEE:
  `link <https://source.codeaurora.org/external/qoriq/qoriq-components/optee_os>`__

* U-Boot:
  `link <https://source.codeaurora.org/external/qoriq/qoriq-components/u-boot>`__

* RCW:
  `link <https://source.codeaurora.org/external/qoriq/qoriq-components/rcw>`__

* ddr-phy-binary (if FIP-DDR needed):
  `link <https:://github.com/NXP/ddr-phy-binary>`__

* cst (required for TBBR):
  `link <https:://source.codeaurora.org/external/qoriq/qoriq-components/cst>`__

Build Procedure
---------------

* Fetch above repositories into local host.

* Prepare AARCH64 toolchain and set the environment variable.

  .. code:: shell

    export CROSS_COMPILE=.../bin/aarch64-linux-gnu-

* Build RCW. Refer README from the respective cloned folder for more details.

* Build U-boot and OP-TEE firstly, and get binary images, u-boot-dtb.bin and tee.bin.
  For u-boot you can use the <platform>_tfa_defconfig for build.

* Copy/clone the repo "ddr-phy-binary" to the tfa directory for platform needing FIP-DDR.

* Below are the steps to build TF-A images.

Build without BL32
^^^^^^^^^^^^^^^^^^

Build BL2

  .. code:: shell

    make PLAT=<platform> \
      BOOT_MODE=<boot_source> \
      RCW=<rcw_binary> \
      pbl

  Set BOOT_MODE with one of following values:

  * sd: SD card boot
  * emmc: eMMC boot
  * nor: IFC NOR boot
  * nand: IFC NAND boot
  * qspi: QSPI NOR boot
  * flexspi_nor: FlexSPI NOR boot

Build FIP

  .. code:: shell

    make PLAT=<platform> \
      BL33=<u-boot_binary> \
      fip

Build with BL32
^^^^^^^^^^^^^^^

Build BL2

  .. code:: shell

    make PLAT=<platform> \
      BOOT_MODE=<boot_source> \
      RCW=<rcw_binary> \
      pbl

  Set BOOT_MODE with one of following values:

  * sd: SD card boot
  * emmc: eMMC boot
  * nor: IFC NOR boot
  * nand: IFC NAND boot
  * qspi: QSPI NOR boot
  * flexspi_nor: FlexSPI NOR boot

Build FIP

  .. code:: shell

    make PLAT=<platform> \
      BL32=<tee_binary> SPD=opteed \
      BL33=<u-boot_binary> \
      fip

Build FIP-DDR (if needed)
^^^^^^^^^^^^^^^^^^^^^^^^^

  .. code:: shell

    make PLAT=<platform> fip-ddr

Binaries
========

+-------------+-------------------------+---------+-----------------+
| Boot Type   | BL2                     | FIP     | FIP-DDR         |
+=============+=========================+=========+=================+
| Normal Boot | bl2_<boot_mode>.pbl     | fip.bin | ddr_fip.bin     |
+-------------+-------------------------+---------+-----------------+
| TBBR Boot   | bl2_<boot_mode>_sec.pbl | fip.bin | ddr_fip_sec.bin |
+-------------+-------------------------+---------+-----------------+

Refer nxp-ls-tbbr.rst for detailed user steps.

Deploy
======

The size in the standard u-boot commands for copy to nor, qspi, nand or sd
should be modified based on the binary size of the image to be copied.

* Deploy images on FlexSPI-NOR flash alt bank from u-boot prompt.

  .. code:: shell

    => tftp 82000000 $path/bl2_flexspi_nor.pbl
    => i2c mw 66 50 20;sf probe 0:0; sf erase 0 +$filesize; sf write 0x82000000 0x0 $filesize

    => tftp 82000000  $path/fip.bin
    => i2c mw 66 50 20;sf probe 0:0; sf erase 0x100000 +$filesize; sf write 0x82000000 0x100000 $filesize

  Next step is valid for platform where FIP-DDR is needed.

  .. code:: shell

    => tftp 82000000 $path/ddr_fip.bin
    => i2c mw 66 50 20; sf probe 0:0; sf erase 0x800000 +$filesize; sf write 0x82000000 0x800000 $filesize

  Then change to alt bank and boot up.

  .. code:: shell

    => qixisreset altbank

* Deploy images on SD/eMMC from u-boot prompt.

  Block number = (byte sizes / 512)

  For eMMC

  .. code:: shell

    => mmc dev 1

  For SD

  .. code:: shell

    => mmc dev 0

  Rest of the commands are same

  .. code:: shell

    => tftp 82000000 $path/bl2_<sd_or_emmc>.pbl
    => mmc write 82000000 8 <block_number>

    => tftp 82000000 $path/fip.bin
    => mmc write 82000000 0x800 <block_number>

  Next step is valid for platform that needs FIP-DDR.

  .. code:: shell

    => tftp 82000000 $path/ddr_fip.bin
    => mmc write 82000000 0x4000 <block_number>

.. _LX2160ARDB: https://www.nxp.com/design/qoriq-developer-resources/layerscape-lx2160a-reference-design-board:LX2160A-RDB
.. _LS1046ARDB: https://www.nxp.com/design/qoriq-developer-resources/layerscape-ls1046a-reference-design-board:LS1046A-RDB
