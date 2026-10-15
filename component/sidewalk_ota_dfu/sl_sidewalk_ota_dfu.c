/***************************************************************************//**
 * @file sl_sidewalk_ota_dfu.c
 * @brief sidewalk over-the-air device firmware upgrade component
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <string.h>
#include "sl_sidewalk_log_pal.h"
#include "sid_pal_assert_ifc.h"
#include "sl_sidewalk_ota_dfu.h"
#include "sl_sidewalk_pal_btl_ifc.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#ifndef SL_SID_OTA_DFU_SCRATCH_BUFFER_SIZE
#define SL_SID_OTA_DFU_SCRATCH_BUFFER_SIZE  (4096)
#endif

#define VALID_BLOCK_SIZE_CNT                (8)

#define OTA_DFU_BLE_MTU                     (217) // Amazon hardcoded this for now

#define FILE_ID_INIT_VALUE                  (0xFFFFFFFF)
#define CRC_INIT_VALUE                      (0)

struct ota_dfu_file {
  uint32_t file_id;
  uint32_t file_size;
  uint32_t crc;
  uint8_t scratch_buffer[SL_SID_OTA_DFU_SCRATCH_BUFFER_SIZE];
  struct sid_bulk_data_transfer_buffer *buffer;
};

struct ota_dfu_ctx {
  struct sid_handle *sidewalk_handle;
  struct ota_dfu_file file;
  struct sid_bulk_data_transfer_stats stats;
  struct sid_bulk_data_transfer_params params;
};

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void on_sbdt_transfer_request(const struct sid_bulk_data_transfer_request *const transfer_request,
                                     struct sid_bulk_data_transfer_response *const transfer_response,
                                     void *context);
static void on_sbdt_data_received(const struct sid_bulk_data_transfer_desc *const desc,
                                  const struct sid_bulk_data_transfer_buffer *const buffer,
                                  void *context);
static void on_sbdt_finalize_request(uint32_t file_id, void *context);
static void on_sbdt_cancel_request(uint32_t file_id, void *context);
static void on_sbdt_error(uint32_t file_id, void *context);
static void on_release_scratch_buffer(uint32_t file_id, void *context);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static const uint16_t valid_block_sizes[VALID_BLOCK_SIZE_CNT] = {
  1024, 2048, 3072, 4096, 5120, 6144, 7168, 8192
};
static struct sid_bulk_data_transfer_event_callbacks sbdt_event_callbacks = {
  .on_transfer_request = on_sbdt_transfer_request,
  .on_data_received = on_sbdt_data_received,
  .on_finalize_request = on_sbdt_finalize_request,
  .on_cancel_request = on_sbdt_cancel_request,
  .on_error = on_sbdt_error,
  .on_release_scratch_buffer = on_release_scratch_buffer,
};
static struct sid_bulk_data_transfer_config sbdt_config = {
  .callbacks = &sbdt_event_callbacks,
};
static struct ota_dfu_ctx ctx = {
  .file = { 0 },
  .sidewalk_handle = NULL,
};

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void on_sbdt_transfer_request(const struct sid_bulk_data_transfer_request *const transfer_request,
                                     struct sid_bulk_data_transfer_response *const transfer_response,
                                     void *context)
{
  (void)context;

  SID_PAL_ASSERT(transfer_request != NULL && transfer_response != NULL);

  SL_SID_LOG_PAL_INFO("pal ota: transf req, file_id: %ld, file_sz: 0x%x, file_off: 0x%x, file_desc_sz: 0x%x, frag_sz: 0x%x, min_scratch_buf_sz: 0x%x",
                      transfer_request->file_id,
                      transfer_request->file_size,
                      transfer_request->file_offset,
                      transfer_request->file_descriptor_size,
                      transfer_request->fragment_size,
                      transfer_request->minimum_scratch_buffer_size);

  uint32_t free_space = sid_pal_btl_ifc_get_free_space();
  if (transfer_request->file_size > free_space) {
    SL_SID_LOG_PAL_ERROR("pal ota: file too big, file_sz: 0x%x", transfer_request->file_size);
    transfer_response->reject_reason = SID_BULK_DATA_TRANSFER_REJECT_REASON_FILE_TOO_BIG;
    goto err;
  }

  if (transfer_request->minimum_scratch_buffer_size > SL_SID_OTA_DFU_SCRATCH_BUFFER_SIZE) {
    SL_SID_LOG_PAL_ERROR("pal ota: scratch buf sz err, min_scratch_buf_sz: 0x%x, scratch_buf_sz: 0x%x",
                         transfer_request->minimum_scratch_buffer_size,
                         SL_SID_OTA_DFU_SCRATCH_BUFFER_SIZE);
    transfer_response->reject_reason = SID_BULK_DATA_TRANSFER_REJECT_REASON_GENERIC;
    goto err;
  }

  SL_SID_LOG_PAL_INFO("pal ota: file transf request ok");
  transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_ACCEPT;
  transfer_response->scratch_buffer = ctx.file.scratch_buffer;
  transfer_response->scratch_buffer_size = sizeof(ctx.file.scratch_buffer);
  ctx.file.file_id = transfer_request->file_id;
  ctx.file.file_size = transfer_request->file_size;
  ctx.file.crc = CRC_INIT_VALUE;
  ctx.file.buffer = NULL;
  memset(ctx.file.scratch_buffer, 0, SL_SID_OTA_DFU_SCRATCH_BUFFER_SIZE);
  return;

  err:
  transfer_response->status = SID_BULK_DATA_TRANSFER_ACTION_REJECT;
  transfer_response->scratch_buffer = NULL;
  transfer_response->scratch_buffer_size = 0;
}

static void on_sbdt_data_received(const struct sid_bulk_data_transfer_desc *const desc,
                                  const struct sid_bulk_data_transfer_buffer *const buffer,
                                  void *context)
{
  (void)context;

  SID_PAL_ASSERT(desc != NULL && buffer != NULL);

  if (desc->file_offset == 0) {
    SL_SID_LOG_PAL_INFO("pal ota: file transf start, file_id: %ld", desc->file_id);
  }

  SL_SID_LOG_PAL_INFO("pal ota: data rcvd, file_id: %ld, file_off: 0x%x, link: %ld, buf_sz: 0x%x",
                      desc->file_id,
                      desc->file_offset,
                      desc->link_type,
                      buffer->size);

  uint8_t *tmp = buffer->data;
  for (uint32_t i = 0; i < buffer->size; i += OTA_DFU_BLE_MTU) {
    uint32_t start_idx = i;
    uint32_t end_idx = (i + OTA_DFU_BLE_MTU > buffer->size) ? (buffer->size - 1) : (i + OTA_DFU_BLE_MTU - 1);
    SL_SID_LOG_PAL_INFO("pal ota: buf[%.4u:%.4u] = {0x%.2x, 0x%.2x, ..., 0x%.2x, 0x%.2x}",
                        start_idx,
                        end_idx,
                        *(tmp + start_idx),
                        *(tmp + start_idx + 1),
                        *(tmp + end_idx - 1),
                        *(tmp + end_idx));
  }

  uint32_t prev_crc = ctx.file.crc;
  ctx.file.crc = sid_pal_btl_ifc_crc32_compute(buffer->data, buffer->size, &ctx.file.crc);
  SL_SID_LOG_PAL_INFO("pal ota: crc update, prev_crc: 0x%.8x, cur_crc: 0x%.8x", prev_crc, ctx.file.crc);

  if (ctx.file.file_size == (desc->file_offset + buffer->size)) {
    SL_SID_LOG_PAL_INFO("pal ota: file rcvd, file_id: %ld, file_sz: 0x%x",
                        ctx.file.file_id,
                        ctx.file.file_size);
  } else if (ctx.file.file_size < (desc->file_offset + buffer->size)) {
    SL_SID_LOG_PAL_ERROR("pal ota: file rcv err, file_id: %ld, file_sz: 0x%x, rcvd_file_sz: 0x%x",
                         ctx.file.file_id,
                         ctx.file.file_size,
                         desc->file_offset + buffer->size);
    return;
  } else {
    // still receiving
  }

  // store the file via the rtos if the following call is not permitted in cb context
  bool err = sid_pal_btl_ifc_write(desc->file_offset, buffer->data, buffer->size);
  if (!err) {
    SL_SID_LOG_PAL_ERROR("pal ota: file write err");
    return;
  }

  ctx.file.buffer = (struct sid_bulk_data_transfer_buffer *)buffer;

  // sid_bulk_data_transfer_release_buffer should be called from within the callback
  // this is a known issue so will be fixed by amazon later on
  sl_app_trigger_ota_dfu_release_buffer();
}

static void on_sbdt_finalize_request(uint32_t file_id, void *context)
{
  (void)context;

  bool finalized = sid_pal_btl_ifc_finalize(ctx.file.file_size, ctx.file.crc);
  if (!finalized) {
    SL_SID_LOG_PAL_ERROR("pal ota: finalize failed, file_id: %ld", file_id);
  }

  sid_error_t ret = sid_bulk_data_transfer_finalize(
    ctx.sidewalk_handle,
    ctx.file.file_id,
    (finalized == true) ? SID_BULK_DATA_TRANSFER_FINAL_STATUS_SUCCESS : SID_BULK_DATA_TRANSFER_FINAL_STATUS_FAILURE);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_PAL_ERROR("pal ota: sid_bulk_data_transfer_finalize failed, err: %d", ret);
  }

  if (!finalized || ret != SID_ERROR_NONE) {
    return;
  }

  SL_SID_LOG_PAL_INFO("pal ota: finalize req rcvd, file_id: %ld", file_id);

  sid_pal_btl_ifc_reboot();
}

static void on_sbdt_cancel_request(uint32_t file_id, void *context)
{
  (void)context;
  SL_SID_LOG_PAL_INFO("pal ota: cancel request, file_id: %ld", file_id);
}

static void on_sbdt_error(uint32_t file_id, void *context)
{
  (void)context;
  SL_SID_LOG_PAL_ERROR("pal ota: generic error, file_id: %ld", file_id);
}

static void on_release_scratch_buffer(uint32_t file_id, void *context)
{
  (void)context;
  memset(&ctx.file, 0, sizeof(struct ota_dfu_file));

  SL_SID_LOG_PAL_INFO("pal ota: scratch buf released, file_id: %ld", file_id);
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sid_ota_dfu_init(struct sid_handle *sidewalk_handle)
{
  if (sidewalk_handle == NULL) {
    SL_SID_LOG_PAL_ERROR("pal ota: %s sid not yet inited", __func__);
    return;
  }

  ctx.sidewalk_handle = sidewalk_handle;

  sid_pal_btl_ifc_init();

  sid_error_t ret = sid_bulk_data_transfer_init(&sbdt_config, sidewalk_handle);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_PAL_ERROR("pal ota: sid_bulk_data_transfer_init failed, err: %d", ret);
    return;
  }
  ctx.file.file_id = FILE_ID_INIT_VALUE;
  ctx.file.file_size = 0;
  ctx.file.crc = CRC_INIT_VALUE;
  ctx.file.buffer = NULL;
  memset(ctx.file.scratch_buffer, 0, SL_SID_OTA_DFU_SCRATCH_BUFFER_SIZE);
  memset(&ctx.params, 0, sizeof(struct sid_bulk_data_transfer_params));
  memset(&ctx.stats, 0, sizeof(struct sid_bulk_data_transfer_stats));

  sbdt_event_callbacks.context = NULL; // user data
  SL_SID_LOG_PAL_INFO("pal ota: init ok");
}

void sl_sid_ota_dfu_deinit(struct sid_handle *sidewalk_handle)
{
  if (sidewalk_handle == NULL) {
    SL_SID_LOG_PAL_ERROR("pal ota: %s sid not yet inited", __func__);
    return;
  }

  sid_error_t ret = sid_bulk_data_transfer_deinit(sidewalk_handle);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_PAL_ERROR("pal ota: sid_bulk_data_transfer_deinit failed, err: %d", ret);
    return;
  }
  ctx.sidewalk_handle = NULL;
  SL_SID_LOG_PAL_INFO("pal ota: deinit ok");
}

void sl_sid_ota_dfu_cancel(struct sid_handle *sidewalk_handle)
{
  if (sidewalk_handle == NULL) {
    SL_SID_LOG_PAL_ERROR("pal ota: %s sid not yet inited", __func__);
    return;
  }

  sid_error_t ret = sid_bulk_data_transfer_cancel(sidewalk_handle, ctx.file.file_id, SID_BULK_DATA_TRANSFER_REJECT_REASON_NONE);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_PAL_ERROR("pal ota: sid_bulk_data_transfer_cancel failed, err: %d", ret);
    return;
  }
  SL_SID_LOG_PAL_INFO("pal ota: cancel ok, file_id: %ld", ctx.file.file_id);
}

void sl_sid_ota_dfu_stat(struct sid_handle *sidewalk_handle)
{
  if (sidewalk_handle == NULL) {
    SL_SID_LOG_PAL_ERROR("pal ota: %s sid not yet inited", __func__);
    return;
  }

  sid_error_t ret = sid_bulk_data_transfer_get_transfer_stats(sidewalk_handle, ctx.file.file_id, &ctx.stats);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_PAL_ERROR("pal ota: sid_bulk_data_transfer_get_transfer_stats failed, err: %d", ret);
    return;
  }
  SL_SID_LOG_PAL_INFO("pal ota: file_id: %ld, file_off: 0x%x, progress: %d/100",
                      ctx.file.file_id, ctx.stats.file_offset, ctx.stats.file_progress_percent);
}

void sl_sid_ota_dfu_param(struct sid_handle *sidewalk_handle)
{
  if (sidewalk_handle == NULL) {
    SL_SID_LOG_PAL_ERROR("pal ota: %s sid not yet inited", __func__);
    return;
  }

  sid_error_t ret = sid_bulk_data_transfer_get_transfer_params(sidewalk_handle, ctx.file.file_id, &ctx.params);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_PAL_ERROR("pal ota: sid_bulk_data_transfer_get_transfer_params failed, err: %d", ret);
    return;
  }
  SL_SID_LOG_PAL_INFO("pal ota: file_id: %ld, frag_sz: 0x%x, file_sz: 0x%x, file_desc_sz: 0x%x, min_scratch_buf_sz: 0x%x, scratch_buf_sz: 0x%x",
                      ctx.file.file_id,
                      ctx.params.fragment_size,
                      ctx.params.file_size,
                      ctx.params.file_descriptor_size,
                      ctx.params.minimum_scratch_buffer_size,
                      ctx.params.scratch_buffer_size);
}

void sl_sid_ota_dfu_min_scratch_buf_size(void)
{
  for (uint8_t i = 0; i < VALID_BLOCK_SIZE_CNT; i++) {
    SL_SID_LOG_PAL_INFO("pal ota: min_scratch_buf_size: 0x%x for block_size: 0x%x",
                        valid_block_sizes[i],
                        sid_bulk_data_transfer_compute_min_scratch_buffer_size(valid_block_sizes[i]));
  }
}

void sl_sid_ota_dfu_release_buffer(void)
{
  sid_error_t ret = sid_bulk_data_transfer_release_buffer(ctx.sidewalk_handle, ctx.file.file_id, ctx.file.buffer);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_PAL_ERROR("pal ota: buf release failed, err: %d", ret);
  }

  sl_sid_ota_dfu_stat(ctx.sidewalk_handle);
}
