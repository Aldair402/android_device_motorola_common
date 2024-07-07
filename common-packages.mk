# Copyright (C) 2014 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# TEMPORARY: These libraries are deprecated, not referenced by any AOSP
# nor OSS HAL We don't add a dependency on the vndk variants as those
# end up in /system but require these in /vendor instead:
PRODUCT_PACKAGES += \
    libhwbinder.vendor \
    libhidltransport.vendor

# A/B
PRODUCT_PACKAGES += \
    otapreopt_script \
    update_engine \
    update_engine_client \
    update_engine_sideload \
    update_verifier

## The following modules are included in debuggable builds only.
PRODUCT_PACKAGES_DEBUG += \
    bootctl

# AIDs / For config.fs
PRODUCT_PACKAGES += \
    fs_config_files \
    fs_config_dirs

# Audio
PRODUCT_PACKAGES += \
    audio.bluetooth.default \
    libalsautils \
    libaudiofoundation.vendor \
    libnbaio_mono \
    libtinycompress \
    tinymix

# Camera
PRODUCT_PACKAGES += \
    libcamera_metadata.vendor \
    libexif.vendor \
    libpng.vendor

# Charger
PRODUCT_PACKAGES += \
    charger_res_images

# cURL
PRODUCT_PACKAGES += \
    libcurl.vendor

# Dynamic
ifeq ($(TARGET_USES_DYNAMIC_PARTITIONS),true)
  PRODUCT_PACKAGES += \
      fastbootd
endif

# Expat
PRODUCT_PACKAGES += \
    libexpat.vendor

# FIXME: master: compat for libprotobuf
# See https://android-review.googlesource.com/c/platform/prebuilts/vndk/v28/+/1109518
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full-3.9.1-vendorcompat \
    libprotobuf-cpp-full-vendorcompat

# Gatekeeper
PRODUCT_PACKAGES += \
    libgatekeeper.vendor

# HIDL
PRODUCT_PACKAGES += \
    android.hidl.base@1.0_vendor

# IMS (OSS)
PRODUCT_PACKAGES += \
    extphonelib \
    extphonelib-product \
    extphonelib.xml \
    extphonelib_product.xml \
    telephony-ext \
    ims-ext-common \
    ims_ext_common.xml \
    qti-telephony-hidl-wrapper-prd \
    qti_telephony_hidl_wrapper_prd.xml \
    qti-telephony-utils-prd \
    qti_telephony_utils_prd.xml

# JSON
PRODUCT_PACKAGES += \
    libjsoncpp.vendor

# Media
PRODUCT_PACKAGES += \
    libavservices_minijail.vendor

# MotoActions
PRODUCT_PACKAGES += \
    MotoActions

# Net
PRODUCT_PACKAGES += \
    libnetutils.vendor

# NFC
ifeq ($(call device-has-characteristic,nfc),true)
  PRODUCT_PACKAGES += \
      com.android.nfc_extras \
      libchrome.vendor \
      NfcNci \
      SecureElement \
      Tag
endif

# OSS Time services
PRODUCT_PACKAGES += \
    timekeep \
    TimeKeep \

# Overlays
## Common Overlays
PRODUCT_PACKAGES += \
    CommonFrameworksOverlay \
    CommonSettingsOverlay

## HFR
ifeq ($(call device-has-characteristic, hfr),true)
  PRODUCT_PACKAGES += \
      CommonHFRSettingsOverlay
endif

## Platform Overlays
PRODUCT_PACKAGES += \
    PlatformFrameworksOverlay \
    PlatformSettingsOverlay \
    PlatformSystemUIOverlay

## Device Overlays
PRODUCT_PACKAGES += \
    $(DEVICE)FrameworksOverlay \
    $(DEVICE)SettingsOverlay \
    $(DEVICE)SystemUIOverlay

# Power
PRODUCT_PACKAGES += \
    libpower.vendor

# RIL
PRODUCT_PACKAGES += \
    ims-moto-libs \
    libjson \
    libprotobuf-cpp-full \
    libsensorndkbridge

# Utils
PRODUCT_PACKAGES += \
    libsysutils.vendor \
    libutilscallstack.vendor

# SQLite
PRODUCT_PACKAGES += \
    libsqlite.vendor

# WiFi
PRODUCT_PACKAGES += \
    libcld80211 \
    libwpa_client
