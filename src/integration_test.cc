// Copyright 2021 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "puffin/src/include/puffin/puffdiff.h"
#include "puffin/src/include/puffin/puffpatch.h"
#include "puffin/src/include/puffin/utils.h"
#include "puffin/memory_stream.h"
#include "puffin/src/puffin_stream.h"
#include "puffin/src/unittest_common.h"

namespace puffin {

namespace {
// xxd -i <name>.zip
const Buffer kTestZipA = {
    0x50, 0x4b, 0x03, 0x04, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x79,
    0x0d, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x1c, 0x00, 0x31, 0x55, 0x54, 0x09, 0x00, 0x03,
    0x5c, 0xed, 0x16, 0x61, 0x5c, 0xed, 0x16, 0x61, 0x75, 0x78, 0x0b, 0x00,
    0x01, 0x04, 0x8f, 0x66, 0x05, 0x00, 0x04, 0x53, 0x5f, 0x01, 0x00, 0x50,
    0x4b, 0x01, 0x02, 0x1e, 0x03, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e,
    0x79, 0x0d, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xa4, 0x81, 0x00, 0x00, 0x00, 0x00, 0x31, 0x55, 0x54,
    0x05, 0x00, 0x03, 0x5c, 0xed, 0x16, 0x61, 0x75, 0x78, 0x0b, 0x00, 0x01,
    0x04, 0x8f, 0x66, 0x05, 0x00, 0x04, 0x53, 0x5f, 0x01, 0x00, 0x50, 0x4b,
    0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x47, 0x00,
    0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x00};

const Buffer kTestZipB = {
    0x50, 0x4b, 0x03, 0x04, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x79,
    0x0d, 0x53, 0x4e, 0x81, 0x88, 0x47, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x1c, 0x00, 0x32, 0x55, 0x54, 0x09, 0x00, 0x03,
    0x88, 0xed, 0x16, 0x61, 0x88, 0xed, 0x16, 0x61, 0x75, 0x78, 0x0b, 0x00,
    0x01, 0x04, 0x8f, 0x66, 0x05, 0x00, 0x04, 0x53, 0x5f, 0x01, 0x00, 0x61,
    0x62, 0x63, 0x0a, 0x50, 0x4b, 0x01, 0x02, 0x1e, 0x03, 0x0a, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x26, 0x79, 0x0d, 0x53, 0x4e, 0x81, 0x88, 0x47, 0x04,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa4, 0x81, 0x00, 0x00, 0x00,
    0x00, 0x32, 0x55, 0x54, 0x05, 0x00, 0x03, 0x88, 0xed, 0x16, 0x61, 0x75,
    0x78, 0x0b, 0x00, 0x01, 0x04, 0x8f, 0x66, 0x05, 0x00, 0x04, 0x53, 0x5f,
    0x01, 0x00, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x47, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00};

}  // namespace

class PuffinIntegrationTest : public testing::TestWithParam<PatchAlgorithm> {
 protected:
  PatchAlgorithm getPatchType() { return GetParam(); }
};

TEST_P(PuffinIntegrationTest, PuffinDiffPatchTest) {
  std::vector<BitExtent> src_deflates;
  ASSERT_TRUE(LocateDeflatesInZipArchive(kTestZipA, &src_deflates));

  std::vector<BitExtent> dst_deflates;
  ASSERT_TRUE(LocateDeflatesInZipArchive(kTestZipB, &dst_deflates));

  std::string tmp_file;
  ASSERT_TRUE(MakeTempFile(&tmp_file, nullptr));
  Buffer patch;
  ASSERT_TRUE(PuffDiff(MemoryStream::CreateForRead(kTestZipA),
                       MemoryStream::CreateForRead(kTestZipB), src_deflates,
                       dst_deflates, {bsdiff::CompressorType::kBrotli},
                       getPatchType(), tmp_file, &patch));

  Buffer patched;
  auto src_stream = MemoryStream::CreateForRead(kTestZipA);
  auto dst_stream = MemoryStream::CreateForWrite(&patched);
  ASSERT_TRUE(PuffPatch(MemoryStream::CreateForRead(kTestZipA),
                        MemoryStream::CreateForWrite(&patched), patch.data(),
                        patch.size()));

  ASSERT_EQ(kTestZipB, patched);
}

INSTANTIATE_TEST_CASE_P(TestWithPatchType,
                        PuffinIntegrationTest,
                        testing::Values(PatchAlgorithm::kBsdiff,
                                        PatchAlgorithm::kZucchini));

}  // namespace puffin