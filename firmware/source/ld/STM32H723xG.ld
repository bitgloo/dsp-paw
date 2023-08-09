/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * AXI SRAM     - BSS, Data, Heap.
 * SRAM1        - SIGGEN.
 * SRAM2        - DAC.
 * SRAM4        - ADC.
 * DTCM-RAM     - Process stacks.
 * ITCM-RAM     - STMDSP Algorithm.
 * BCKP SRAM    - None.
 */
MEMORY
{
    flash0 (rx) : org = 0x08000000, len = 1M       /* Flash bank1 + bank2 */
    flash1 (rx) : org = 0x08000000, len = 510K     /* Flash bank 1 */
    flashc (rx) : org = 0x0807F800, len = 2K       /* Unprivileged firmware */
    flash2 (rx) : org = 0x08080000, len = 512K     /* Flash bank 2 */
    flash3 (rx) : org = 0x00000000, len = 0
    flash4 (rx) : org = 0x00000000, len = 0
    flash5 (rx) : org = 0x00000000, len = 0
    flash6 (rx) : org = 0x00000000, len = 0
    flash7 (rx) : org = 0x00000000, len = 0
    ram0   (wx) : org = 0x24000000, len = 320K     /* AXI SRAM */
    ram1   (wx) : org = 0x30000000, len = 16K      /* AHB SRAM1 */
    ram2   (wx) : org = 0x30004000, len = 16K      /* AHB SRAM2 */
    ram3   (wx) : org = 0x38000000, len = 16K      /* AHB SRAM4 */
    ram4   (wx) : org = 0x00000000, len = 0
    ramc   (wx) : org = 0x20000000, len = 64K      /* Unprivileged data */
    ram5   (wx) : org = 0x20010000, len = 64K      /* DTCM-RAM */
    ram6   (wx) : org = 0x00000000, len = 64K      /* ITCM-RAM */
    ram7   (wx) : org = 0x38800000, len = 4K       /* BCKP SRAM */
}

/* For each data/text section two region are defined, a virtual region
   and a load region (_LMA suffix).*/

/* Flash region to be used for exception vectors.*/
REGION_ALIAS("VECTORS_FLASH", flash0);
REGION_ALIAS("VECTORS_FLASH_LMA", flash0);

/* Flash region to be used for constructors and destructors.*/
REGION_ALIAS("XTORS_FLASH", flash0);
REGION_ALIAS("XTORS_FLASH_LMA", flash0);

/* Flash region to be used for code text.*/
REGION_ALIAS("TEXT_FLASH", flash0);
REGION_ALIAS("TEXT_FLASH_LMA", flash0);

/* Flash region to be used for read only data.*/
REGION_ALIAS("RODATA_FLASH", flash0);
REGION_ALIAS("RODATA_FLASH_LMA", flash0);

/* Flash region to be used for various.*/
REGION_ALIAS("VARIOUS_FLASH", flash0);
REGION_ALIAS("VARIOUS_FLASH_LMA", flash0);

/* Flash region to be used for RAM(n) initialization data.*/
REGION_ALIAS("RAM_INIT_FLASH_LMA", flash0);

/* RAM region to be used for Main stack. This stack accommodates the processing
   of all exceptions and interrupts.*/
REGION_ALIAS("MAIN_STACK_RAM", ram5);

/* RAM region to be used for the process stack. This is the stack used by
   the main() function.*/
REGION_ALIAS("PROCESS_STACK_RAM", ram5);

/* RAM region to be used for data segment.*/
REGION_ALIAS("DATA_RAM", ram0);
REGION_ALIAS("DATA_RAM_LMA", flash0);

/* RAM region to be used for BSS segment.*/
REGION_ALIAS("BSS_RAM", ram0);

/* RAM region to be used for the default heap.*/
REGION_ALIAS("HEAP_RAM", ram0);

/* Stack rules inclusion.*/
INCLUDE rules_stacks.ld

SECTIONS
{
    .convdata : ALIGN(4)
    {
        *(.convdata)
        . = ALIGN(4);
    } > ramc

    .stacks : ALIGN(4)
    {
        *(.stacks)
        . = ALIGN(4);
    } > ram5

    .convcode : ALIGN(4)
    {
        *(.convcode)
        . = ALIGN(4);
    } > flashc
}

/* Code rules inclusion.*/
INCLUDE rules_code.ld

/* Data rules inclusion.*/
INCLUDE rules_data.ld

/* Memory rules inclusion.*/
INCLUDE rules_memory.ld

