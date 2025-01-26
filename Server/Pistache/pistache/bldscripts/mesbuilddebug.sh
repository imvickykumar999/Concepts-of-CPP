#!/bin/bash

#
# SPDX-FileCopyrightText: 2024 Duncan Greatwood
#
# SPDX-License-Identifier: Apache-2.0
#

# Execute this script from the parent directory by invoking:
#   bldscripts/mesbuilddebug.sh

MY_SCRIPT_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source $MY_SCRIPT_DIR/helpers/mesdebugsetdirvars.sh
source $MY_SCRIPT_DIR/helpers/adjbuilddirformesbuild.sh

if [ -e "${MESON_BUILD_DIR}" ]
then
    echo "Using existing build dir ${MESON_BUILD_DIR}"
else
    meson setup ${MESON_BUILD_DIR} \
    --buildtype=debug \
    -DPISTACHE_USE_SSL=true \
    -DPISTACHE_BUILD_EXAMPLES=true \
    -DPISTACHE_BUILD_TESTS=true \
    -DPISTACHE_BUILD_DOCS=false \
    -DPISTACHE_USE_CONTENT_ENCODING_DEFLATE=true \
    -DPISTACHE_USE_CONTENT_ENCODING_BROTLI=true \
    -DPISTACHE_USE_CONTENT_ENCODING_ZSTD=true \
    -DPISTACHE_DEBUG=true \
    --prefix="${MESON_PREFIX_DIR}" \
#    -DPISTACHE_FORCE_LIBEVENT=true

fi

meson compile -C ${MESON_BUILD_DIR}
