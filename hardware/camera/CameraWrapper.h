/*
 * Copyright (C) 2017, The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/String8.h>
#include <utils/Mutex.h>
#include <hardware/hardware.h>
#include <hardware/camera.h>
#include <system/camera_metadata.h>
#include <CameraParameters.h>

using ::android::hardware::camera::common::V1_0::helper::CameraParameters;
using ::android::hardware::camera::common::V1_0::helper::Size;

static android::Mutex gCameraWrapperLock;

/*
 * Strip Samsung vendor tag entries (tag >= 0x80000000) from camera metadata.
 * These corrupt Java-side CameraCharacteristics parsing when the vendor tag
 * descriptor doesn't match the actual metadata entries.
 */
static camera_metadata_t* sanitize_camera_metadata(const camera_metadata_t *src) {
    if (!src) return NULL;
    size_t src_count = get_camera_metadata_entry_count(src);
    size_t src_data = get_camera_metadata_data_count(src);
    camera_metadata_t *dst = allocate_camera_metadata(src_count + 16, src_data + 256);
    if (!dst) return NULL;
    for (size_t i = 0; i < src_count; i++) {
        camera_metadata_entry_t entry;
        if (get_camera_metadata_entry((camera_metadata_t*)src, i, &entry) != 0)
            continue;
        if (entry.tag >= 0x80000000u)
            continue;
        add_camera_metadata_entry(dst, entry.tag, entry.data.u8, entry.count);
    }
    return dst;
}

