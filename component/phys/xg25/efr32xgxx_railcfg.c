/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config  -  DO NOT EDIT
 *   Radio Configurator Version: 2302.4.3
 *   RAIL Adapter Version: 2.4.31
 *   RAIL Compatibility: 2.x
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_device.h"
#include "rail_types.h"

uint32_t RAILCb_CalcSymbolRate(RAIL_Handle_t railHandle)
{
  (void) railHandle;
  return 0U;
}

uint32_t RAILCb_CalcBitRate(RAIL_Handle_t railHandle)
{
  (void) railHandle;
  return 0U;
}

void RAILCb_ConfigFrameTypeLength(RAIL_Handle_t railHandle,
                                  const RAIL_FrameType_t *frameType)
{
  (void) railHandle;
  (void) frameType;
}

static const uint8_t irCalConfig[] = {
  20, 41, 2, 0, 0, 57, 19, 0, 0, 0, 1, 0, 2, 100, 0, 1, 1, 47, 0, 0, 7
};

static const uint8_t txIrCalConfig[5] = {
  0x00, 0x03, 5, 5, 5
};

static const uint32_t rffpllConfig[] = {
  6558726, 325000000, 97500000
};

const RAIL_RffpllConfig_t *radioConfigRffpllConfig = (RAIL_RffpllConfig_t *) rffpllConfig;

#if RAIL_SUPPORTS_HFXO_COMPENSATION
static const uint16_t modemTxCompensation[33] = {
  0x2b93, 0xce67, 0x273a, 0x277f, 0xd881, 0x3121, 0x2397, 0xe2ab, 0x4120, 0x1f7f, 0xecff, 0x611e, 0x1b7f, 0xf602, 0xc3, 0x177f, 0x00fe, 0x60bd, 0x137f, 0x0a02, 0xc3, 0xf7f, 0x14ff, 0x611f, 0xb97, 0x1eab, 0x4121, 0x77f, 0x2881, 0x3122, 0x393, 0x3267, 0x273b
};
#endif

static const int32_t timingConfig[] = {
  140000, 140000, 0
};

static const uint8_t hfxoRetimingConfigEntries[] = {
  1, 0, 0, 0, 0xc0, 0x17, 0x53, 0x02, 4, 12, 0, 0, 0xe0, 0x02, 0, 0, 0, 0, 0x3c, 0x03, 1, 2, 5, 4, 0x98, 0x03, 1, 2, 5, 5, 0xf4, 0x03, 1, 2, 6, 5
};

static RAIL_ChannelConfigEntryAttr_t channelConfigEntryAttr = {
#if RAIL_SUPPORTS_OFDM_PA
  {
#ifdef RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
    { 0xFFFFFFFFUL, 0xFFFFFFFFUL, },
#else
    { 0xFFFFFFFFUL },
#endif // RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
    { 0xFFFFFFFFUL, 0xFFFFFFFFUL }
  }
#else // RAIL_SUPPORTS_OFDM_PA
#ifdef RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
  { 0xFFFFFFFFUL, 0xFFFFFFFFUL, },
#else
  { 0xFFFFFFFFUL },
#endif // RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
#endif // RAIL_SUPPORTS_OFDM_PA
};

static const uint32_t phyInfo[] = {
  15UL,
  0x002C71C7UL, // 44.44444444444444
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig,
  0x00000000UL,
  0UL,
  0UL,
  50000UL,
  0x00F00101UL,
  0x07101570UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  49999UL,
  (uint32_t) rffpllConfig,
  (uint32_t) txIrCalConfig,
#if RAIL_SUPPORTS_HFXO_COMPENSATION
  (uint32_t) modemTxCompensation,
#else
  (uint32_t) NULL,
#endif
};

const uint32_t phy50kbps_modemConfigBase[] = {
  0x10018058UL, 0xBF1FF07FUL,
  0x30018058UL, 0x40000000UL,
  0x0102400CUL, 0x0011B10CUL,
  /*    4010 */ 0x00004800UL,
  0x01024020UL, 0x00000000UL,
  /*    4024 */ 0x00000000UL,
  0x01074030UL, 0x00000825UL,
  /*    4034 */ 0x00000100UL,
  /*    4038 */ 0x000000FFUL,
  /*    403C */ 0x00010341UL,
  /*    4040 */ 0x00000000UL,
  /*    4044 */ 0x00006000UL,
  /*    4048 */ 0x030007A0UL,
  0x01014050UL, 0x0000000BUL,
  0x0102405CUL, 0x00000D0FUL,
  /*    4060 */ 0x00000101UL,
  0x010140A8UL, 0x00000007UL,
  0x010440BCUL, 0x00000000UL,
  /*    40C0 */ 0x00000000UL,
  /*    40C4 */ 0x00000000UL,
  /*    40C8 */ 0x00000000UL,
  0x01044108UL, 0x00004001UL,
  /*    410C */ 0x000000FFUL,
  /*    4110 */ 0x00004101UL,
  /*    4114 */ 0x000001FFUL,
  0x01014184UL, 0x00000001UL,
  0x1101C020UL, 0x0007F800UL,
  0x3101C020UL, 0x002801FEUL,
  0x0109C024UL, 0x00001300UL,
  /*    C028 */ 0x03B380ECUL,
  /*    C02C */ 0x51407543UL,
  /*    C030 */ 0xF8000FA0UL,
  /*    C034 */ 0x00004030UL,
  /*    C038 */ 0x0007AAA8UL,
  /*    C03C */ 0x00000000UL,
  /*    C040 */ 0x00000000UL,
  /*    C044 */ 0x00000000UL,
  0x0114C054UL, 0x00302187UL,
  /*    C058 */ 0xE68B0095UL,
  /*    C05C */ 0x000001BFUL,
  /*    C060 */ 0x867F7052UL,
  /*    C064 */ 0x0000008BUL,
  /*    C068 */ 0x0002C688UL,
  /*    C06C */ 0x000004A0UL,
  /*    C070 */ 0x000013BAUL,
  /*    C074 */ 0x00190000UL,
  /*    C078 */ 0x0065003AUL,
  /*    C07C */ 0x00DA0096UL,
  /*    C080 */ 0x02BA01C0UL,
  /*    C084 */ 0x05B003D6UL,
  /*    C088 */ 0x09EA07B2UL,
  /*    C08C */ 0x10FE0CFAUL,
  /*    C090 */ 0x1D5A16D2UL,
  /*    C094 */ 0x3178269AUL,
  /*    C098 */ 0x36DA3FFFUL,
  /*    C09C */ 0x3FFF39D0UL,
  /*    C0A0 */ 0x00003FFFUL,
  0x0105C0A8UL, 0x15724BBDUL,
  /*    C0AC */ 0x0518A311UL,
  /*    C0B0 */ 0x76543210UL,
  /*    C0B4 */ 0x00000A98UL,
  /*    C0B8 */ 0x00000000UL,
  0x0104C0CCUL, 0x000001FEUL,
  /*    C0D0 */ 0x00000000UL,
  /*    C0D4 */ 0x000A0001UL,
  /*    C0D8 */ 0x00280001UL,
  0x0101C100UL, 0x00002800UL,
  0x0102C110UL, 0x00010100UL,
  /*    C114 */ 0x000000C8UL,
  0x02010008UL, 0x00000704UL,
  0x02010018UL, 0x00000000UL,
  0x02010020UL, 0x00008408UL,
  0x02024040UL, 0x40500000UL,
  /*    4044 */ 0x00000000UL,
  0x0209404CUL, 0x04000000UL,
  /*    4050 */ 0x0082C02FUL,
  /*    4054 */ 0x20000000UL,
  /*    4058 */ 0x00000000UL,
  /*    405C */ 0x03000000UL,
  /*    4060 */ 0x40001000UL,
  /*    4064 */ 0x00000000UL,
  /*    4068 */ 0x00FE60BDUL,
  /*    406C */ 0x00000C41UL,
  0x02194074UL, 0x00200012UL,
  /*    4078 */ 0x00007209UL,
  /*    407C */ 0x00000000UL,
  /*    4080 */ 0x00000E40UL,
  /*    4084 */ 0x00000000UL,
  /*    4088 */ 0x001A037FUL,
  /*    408C */ 0x62040000UL,
  /*    4090 */ 0x00000000UL,
  /*    4094 */ 0x1A060100UL,
  /*    4098 */ 0x54544F3AUL,
  /*    409C */ 0x00000000UL,
  /*    40A0 */ 0x00000000UL,
  /*    40A4 */ 0x00000000UL,
  /*    40A8 */ 0x00000000UL,
  /*    40AC */ 0x00000000UL,
  /*    40B0 */ 0x00000000UL,
  /*    40B4 */ 0x00000000UL,
  /*    40B8 */ 0x00000000UL,
  /*    40BC */ 0x00000000UL,
  /*    40C0 */ 0x00000000UL,
  /*    40C4 */ 0x00000000UL,
  /*    40C8 */ 0x00000000UL,
  /*    40CC */ 0x00000000UL,
  /*    40D0 */ 0x00000000UL,
  /*    40D4 */ 0x00000000UL,
  0x020140E0UL, 0x00000281UL,
  0x02074120UL, 0x00000000UL,
  /*    4124 */ 0x078304FFUL,
  /*    4128 */ 0x3AC81388UL,
  /*    412C */ 0x0C6606FFUL,
  /*    4130 */ 0x078304FFUL,
  /*    4134 */ 0x03FF1388UL,
  /*    4138 */ 0xF00A20BCUL,
  0x02054140UL, 0x40AD7103UL,
  /*    4144 */ 0x904E0000UL,
  /*    4148 */ 0x4239CFB0UL,
  /*    414C */ 0x00403B8BUL,
  /*    4150 */ 0x800003C0UL,
  0x02024158UL, 0x00000000UL,
  /*    415C */ 0x0000FDFFUL,
  0x02014164UL, 0x0000010CUL,
  0x020B416CUL, 0x40000000UL,
  /*    4170 */ 0x00000000UL,
  /*    4174 */ 0x00000000UL,
  /*    4178 */ 0x00000000UL,
  /*    417C */ 0x00000000UL,
  /*    4180 */ 0x00000000UL,
  /*    4184 */ 0x00000101UL,
  /*    4188 */ 0x00000000UL,
  /*    418C */ 0x00000000UL,
  /*    4190 */ 0x00000000UL,
  /*    4194 */ 0x00000000UL,
  0x020141A4UL, 0x00000000UL,
  0x020C41B0UL, 0x00000000UL,
  /*    41B4 */ 0xC02ED568UL,
  /*    41B8 */ 0x00000000UL,
  /*    41BC */ 0x00000000UL,
  /*    41C0 */ 0x003C0000UL,
  /*    41C4 */ 0x0006AAAAUL,
  /*    41C8 */ 0x00000000UL,
  /*    41CC */ 0x00000000UL,
  /*    41D0 */ 0x55555555UL,
  /*    41D4 */ 0x80AC81E6UL,
  /*    41D8 */ 0x00020007UL,
  /*    41DC */ 0x00000000UL,
  0x02054224UL, 0x8000FFFFUL,
  /*    4228 */ 0x00000000UL,
  /*    422C */ 0x00000000UL,
  /*    4230 */ 0x00000000UL,
  /*    4234 */ 0x00000000UL,
  0x0201423CUL, 0x00000000UL,
  0x02034244UL, 0x90000014UL,
  /*    4248 */ 0x00000000UL,
  /*    424C */ 0x04060008UL,
  0x020F4330UL, 0x00000000UL,
  /*    4334 */ 0x00000000UL,
  /*    4338 */ 0x00000000UL,
  /*    433C */ 0x00000000UL,
  /*    4340 */ 0x00000000UL,
  /*    4344 */ 0x00000000UL,
  /*    4348 */ 0x00000000UL,
  /*    434C */ 0x00000000UL,
  /*    4350 */ 0x00000000UL,
  /*    4354 */ 0x00000000UL,
  /*    4358 */ 0x00000000UL,
  /*    435C */ 0x38000000UL,
  /*    4360 */ 0x00000000UL,
  /*    4364 */ 0x00000000UL,
  /*    4368 */ 0x58FF0000UL,
  0x02014400UL, 0x000000E0UL,
  0x02018010UL, 0x00000003UL,
  0x02028038UL, 0x00101570UL,
  /*    803C */ 0x00000003UL,
  0x0203809CUL, 0x00000000UL,
  /*    80A0 */ 0x0003B870UL,
  /*    80A4 */ 0x0003B870UL,
  0x120180A8UL, 0x000001F6UL,
  0x320180A8UL, 0x01014201UL,
  0x120180ACUL, 0x000001F6UL,
  0x320180ACUL, 0x01014201UL,
  0x020280B0UL, 0x02000300UL,
  /*    80B4 */ 0x02000300UL,
  0x03030098UL, 0x00000000UL,
  /*    009C */ 0x04000C00UL,
  /*    00A0 */ 0x0000044CUL,
  0x030200D8UL, 0xAA400005UL,
  /*    00DC */ 0x00000188UL,
  0x130100ECUL, 0x00000FE0UL,
  0x330100ECUL, 0x5351200DUL,
  0x030100F0UL, 0x0000052BUL,
  0x03010100UL, 0x00000110UL,
  0x13010104UL, 0x00000000UL,
  0x33010104UL, 0x00000110UL,
  0x13010110UL, 0x000FFF00UL,
  0x33010110UL, 0x42000002UL,
  0x1301012CUL, 0x001FFC00UL,
  0x3301012CUL, 0x008002E9UL,
  0x03010140UL, 0x0000003FUL,
  0x13010150UL, 0x0001C000UL,
  0x33010150UL, 0x00A200C8UL,
  0x03020168UL, 0x00060010UL,
  /*    016C */ 0x000C0000UL,
  0x03010174UL, 0x01900169UL,
  0x13010178UL, 0x001C0000UL,
  0x33010178UL, 0x1FE00440UL,
  0x13010180UL, 0x00000779UL,
  0x33010180UL, 0x00000006UL,
  0x03020188UL, 0x00000090UL,
  /*    018C */ 0x00000000UL,
  0x030101E4UL, 0x00045220UL,
  0x03010208UL, 0x00200008UL,
  0x03010210UL, 0x00001100UL,
  0x13010400UL, 0x00000008UL,
  0x05120100UL, 0x0000005AUL,
  /*    0104 */ 0x0809AC31UL,
  /*    0108 */ 0x00000000UL,
  /*    010C */ 0x046EC3D5UL,
  /*    0110 */ 0x000E599AUL,
  /*    0114 */ 0x00694F64UL,
  /*    0118 */ 0x004ADF9DUL,
  /*    011C */ 0x0E6B035CUL,
  /*    0120 */ 0x01E2F5F3UL,
  /*    0124 */ 0x36172668UL,
  /*    0128 */ 0x04AFEFF2UL,
  /*    012C */ 0x000F2901UL,
  /*    0130 */ 0x00091239UL,
  /*    0134 */ 0x00B66E73UL,
  /*    0138 */ 0x04B0C6FFUL,
  /*    013C */ 0x0F887A78UL,
  /*    0140 */ 0x20E11CE3UL,
  /*    0144 */ 0x80000000UL,
  0x05010180UL, 0x00506DC2UL,
  0x05020200UL, 0x00145463UL,
  /*    0204 */ 0x00000000UL,
  0x05124100UL, 0x00000000UL,
  /*    4104 */ 0x00000000UL,
  /*    4108 */ 0x00000000UL,
  /*    410C */ 0x00000000UL,
  /*    4110 */ 0x00000000UL,
  /*    4114 */ 0x00000000UL,
  /*    4118 */ 0x00000000UL,
  /*    411C */ 0x00000000UL,
  /*    4120 */ 0x00000000UL,
  /*    4124 */ 0x00000000UL,
  /*    4128 */ 0x00000000UL,
  /*    412C */ 0x00000000UL,
  /*    4130 */ 0x00000000UL,
  /*    4134 */ 0x00000000UL,
  /*    4138 */ 0x00000000UL,
  /*    413C */ 0x00000000UL,
  /*    4140 */ 0x00000000UL,
  /*    4144 */ 0x00000000UL,
  0x05014154UL, 0x00000000UL,
  0x05014180UL, 0x00000000UL,
  0x05024200UL, 0x00000030UL,
  /*    4204 */ 0x00000000UL,
  0x050B8100UL, 0x00000000UL,
  /*    8104 */ 0x00000000UL,
  /*    8108 */ 0x00000000UL,
  /*    810C */ 0x00000000UL,
  /*    8110 */ 0x00000000UL,
  /*    8114 */ 0x00000000UL,
  /*    8118 */ 0x00000000UL,
  /*    811C */ 0x00000000UL,
  /*    8120 */ 0x00000000UL,
  /*    8124 */ 0x00000000UL,
  /*    8128 */ 0x00000000UL,
  0x06016FF4UL, 0x00000004UL,
  0x06016FFCUL, (uint32_t) &phyInfo,
  0x0701FC00UL, 0x00000000UL,
  0x0701FC00UL, 0x00000000UL,
  0x0701FC00UL, 0x00000000UL,
  0x0701FC00UL, 0x00000000UL,
  0x0702FC00UL, 0x00000000UL,
  /*    FC04 */ 0x00000000UL,
  0x0701FC04UL, 0x00000000UL,
  0x0701FC04UL, 0x00000000UL,
  0x0701FC04UL, 0x00000000UL,
  0x0702FC04UL, 0x00000000UL,
  /*    FC08 */ 0x00000000UL,
  0x0701FC08UL, 0x00000000UL,
  0x0701FC08UL, 0x00000000UL,
  0x0701FC08UL, 0x00000000UL,
  0x0707FC08UL, 0x00000000UL,
  /*    FC0C */ 0x00000000UL,
  /*    FC10 */ 0x00000000UL,
  /*    FC14 */ 0x00000014UL,
  /*    FC18 */ 0x00000000UL,
  /*    FC1C */ 0x00000000UL,
  /*    FC20 */ 0x00000000UL,
  0x0701FC30UL, 0x00000000UL,
  0x0701FC30UL, 0x00000000UL,
  0x0702FC30UL, 0x00000000UL,
  /*    FC34 */ 0x00000000UL,
  0x0702FC34UL, 0x00000000UL,
  /*    FC38 */ 0x00000000UL,
  0x0702FC38UL, 0x00000000UL,
  /*    FC3C */ 0x00000000UL,
  0x0702FC3CUL, 0x00000000UL,
  /*    FC40 */ 0x00000000UL,
  0x0702FC40UL, 0x00000000UL,
  /*    FC44 */ 0x00000000UL,
  0x0702FC44UL, 0x00000000UL,
  /*    FC48 */ 0x00000000UL,
  0x0702FC48UL, 0x00000000UL,
  /*    FC4C */ 0x00000000UL,
  0x0702FC4CUL, 0x00000000UL,
  /*    FC50 */ 0x00000000UL,
  0x0702FC50UL, 0x00000000UL,
  /*    FC54 */ 0x00000000UL,
  0x0702FC54UL, 0x00000000UL,
  /*    FC58 */ 0x00000080UL,
  0x0702FC58UL, 0x00000080UL,
  /*    FC5C */ 0x00000000UL,
  0x0702FC5CUL, 0x00000000UL,
  /*    FC60 */ 0x00000000UL,
  0x0702FC60UL, 0x00000000UL,
  /*    FC64 */ 0x00000000UL,
  0x0706FC64UL, 0x00000000UL,
  /*    FC68 */ 0x00000000UL,
  /*    FC6C */ 0x00000000UL,
  /*    FC70 */ 0x00000000UL,
  /*    FC74 */ 0x00000000UL,
  /*    FC78 */ 0x00000000UL,
  0x070BFC80UL, 0x10001000UL,
  /*    FC84 */ 0x00000080UL,
  /*    FC88 */ 0x00000000UL,
  /*    FC8C */ 0x00000000UL,
  /*    FC90 */ 0x00000000UL,
  /*    FC94 */ 0x00000000UL,
  /*    FC98 */ 0x00000000UL,
  /*    FC9C */ 0x00000000UL,
  /*    FCA0 */ 0x00000000UL,
  /*    FCA4 */ 0x00000000UL,
  /*    FCA8 */ 0x00000000UL,
  0xFFFFFFFFUL,
};

const RAIL_ChannelConfigEntry_t phy50kbps_channels[] = {
  {
    .phyConfigDeltaAdd = NULL,
    .baseFrequency = 902200000,
    .channelSpacing = 200000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 68,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = NULL,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfig_t phy50kbps_channelConfig = {
  .phyConfigBase = phy50kbps_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = phy50kbps_channels,
  .length = 1U,
  .signature = 0UL
};

const RAIL_ChannelConfig_t *efr32xgxx_channelConfigs[] = {
  &phy50kbps_channelConfig,
  NULL
};
