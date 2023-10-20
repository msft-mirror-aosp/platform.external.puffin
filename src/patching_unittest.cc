// Copyright 2017 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "puffin/memory_stream.h"
#include "puffin/src/include/puffin/common.h"
#include "puffin/src/include/puffin/puffdiff.h"
#include "puffin/src/include/puffin/puffpatch.h"
#include "puffin/src/include/puffin/utils.h"
#include "puffin/src/logging.h"
#include "puffin/src/puffin_stream.h"
#include "puffin/src/unittest_common.h"

#define PRINT_SAMPLE 0  // Set to 1 if you want to print the generated samples.

using std::string;
using std::vector;

namespace puffin {

namespace {

#if PRINT_SAMPLE
// Print an array into hex-format to the output. This can be used to create
// static arrays for unit testing of the puffer/huffer.
void PrintArray(const string& name, const Buffer& array) {
  std::cout << "const Buffer " << name << " = {" << std::endl << " ";
  for (size_t idx = 0; idx < array.size(); idx++) {
    std::cout << " 0x" << std::hex << std::uppercase << std::setfill('0')
              << std::setw(2) << uint(array[idx]);
    if (idx == array.size() - 1) {
      std::cout << std::dec << "};" << std::endl;
      return;
    }
    std::cout << ",";
    if ((idx + 1) % 12 == 0) {
      std::cout << std::endl << " ";
    }
  }
}
#endif

const Buffer kPatch1To2 = {
    0x50, 0x55, 0x46, 0x31, 0x00, 0x00, 0x00, 0x51, 0x08, 0x01, 0x12, 0x27,
    0x0A, 0x04, 0x08, 0x10, 0x10, 0x32, 0x0A, 0x04, 0x08, 0x50, 0x10, 0x0A,
    0x0A, 0x04, 0x08, 0x60, 0x10, 0x12, 0x12, 0x04, 0x08, 0x10, 0x10, 0x58,
    0x12, 0x04, 0x08, 0x78, 0x10, 0x28, 0x12, 0x05, 0x08, 0xA8, 0x01, 0x10,
    0x38, 0x18, 0x1F, 0x1A, 0x24, 0x0A, 0x02, 0x10, 0x32, 0x0A, 0x04, 0x08,
    0x48, 0x10, 0x50, 0x0A, 0x05, 0x08, 0x98, 0x01, 0x10, 0x12, 0x12, 0x02,
    0x10, 0x58, 0x12, 0x04, 0x08, 0x70, 0x10, 0x58, 0x12, 0x05, 0x08, 0xC8,
    0x01, 0x10, 0x38, 0x18, 0x21, 0x42, 0x53, 0x44, 0x46, 0x32, 0x01, 0x01,
    0x01, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x42, 0x5A, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0xD1,
    0x20, 0xBB, 0x7E, 0x00, 0x00, 0x03, 0x60, 0x40, 0x78, 0x0E, 0x08, 0x00,
    0x40, 0x00, 0x20, 0x00, 0x31, 0x06, 0x4C, 0x40, 0x92, 0x8F, 0x46, 0xA7,
    0xA8, 0xE0, 0xF3, 0xD6, 0x21, 0x12, 0xF4, 0xBC, 0x43, 0x32, 0x1F, 0x17,
    0x72, 0x45, 0x38, 0x50, 0x90, 0xD1, 0x20, 0xBB, 0x7E, 0x42, 0x5A, 0x68,
    0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0xF1, 0x20, 0x5F, 0x0D, 0x00,
    0x00, 0x02, 0x41, 0x15, 0x42, 0x08, 0x20, 0x00, 0x40, 0x00, 0x00, 0x02,
    0x40, 0x00, 0x20, 0x00, 0x22, 0x3D, 0x23, 0x10, 0x86, 0x03, 0x96, 0x54,
    0x11, 0x16, 0x5F, 0x17, 0x72, 0x45, 0x38, 0x50, 0x90, 0xF1, 0x20, 0x5F,
    0x0D, 0x42, 0x5A, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0x07,
    0xD4, 0xCB, 0x6E, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x20, 0x00,
    0x21, 0x18, 0x46, 0x82, 0xEE, 0x48, 0xA7, 0x0A, 0x12, 0x00, 0xFA, 0x99,
    0x6D, 0xC0};

const Buffer kPatch2To1 = {
    0x50, 0x55, 0x46, 0x31, 0x00, 0x00, 0x00, 0x51, 0x08, 0x01, 0x12, 0x24,
    0x0A, 0x02, 0x10, 0x32, 0x0A, 0x04, 0x08, 0x48, 0x10, 0x50, 0x0A, 0x05,
    0x08, 0x98, 0x01, 0x10, 0x12, 0x12, 0x02, 0x10, 0x58, 0x12, 0x04, 0x08,
    0x70, 0x10, 0x58, 0x12, 0x05, 0x08, 0xC8, 0x01, 0x10, 0x38, 0x18, 0x21,
    0x1A, 0x27, 0x0A, 0x04, 0x08, 0x10, 0x10, 0x32, 0x0A, 0x04, 0x08, 0x50,
    0x10, 0x0A, 0x0A, 0x04, 0x08, 0x60, 0x10, 0x12, 0x12, 0x04, 0x08, 0x10,
    0x10, 0x58, 0x12, 0x04, 0x08, 0x78, 0x10, 0x28, 0x12, 0x05, 0x08, 0xA8,
    0x01, 0x10, 0x38, 0x18, 0x1F, 0x42, 0x53, 0x44, 0x46, 0x32, 0x01, 0x01,
    0x01, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x42, 0x5A, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0x3D,
    0xBD, 0x08, 0x91, 0x00, 0x00, 0x01, 0xE0, 0x40, 0x5C, 0x0A, 0x40, 0x00,
    0x40, 0x00, 0x20, 0x00, 0x31, 0x0C, 0x08, 0x23, 0xD2, 0x34, 0xD1, 0xB1,
    0x73, 0x60, 0x44, 0x54, 0xE4, 0xFC, 0x5D, 0xC9, 0x14, 0xE1, 0x42, 0x40,
    0xF6, 0xF4, 0x22, 0x44, 0x42, 0x5A, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26,
    0x53, 0x59, 0x41, 0x62, 0x2E, 0xF0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40,
    0x20, 0x20, 0x00, 0x21, 0x00, 0x82, 0x83, 0x17, 0x72, 0x45, 0x38, 0x50,
    0x90, 0x41, 0x62, 0x2E, 0xF0, 0x42, 0x5A, 0x68, 0x39, 0x31, 0x41, 0x59,
    0x26, 0x53, 0x59, 0xE0, 0x20, 0x04, 0x57, 0x00, 0x00, 0x04, 0x76, 0x50,
    0xE0, 0x00, 0x20, 0x00, 0x10, 0x00, 0x04, 0x00, 0x02, 0x00, 0x20, 0x00,
    0x40, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x21, 0xA1, 0xA3, 0x10, 0x83, 0x26,
    0x21, 0x5E, 0xB2, 0x69, 0xAC, 0x70, 0x60, 0x53, 0xC5, 0xDC, 0x91, 0x4E,
    0x14, 0x24, 0x38, 0x08, 0x01, 0x15, 0xC0};

const Buffer kPatch1ToEmpty = {
    0x50, 0x55, 0x46, 0x31, 0x00, 0x00, 0x00, 0x2D, 0x08, 0x01, 0x12, 0x27,
    0x0A, 0x04, 0x08, 0x10, 0x10, 0x32, 0x0A, 0x04, 0x08, 0x50, 0x10, 0x0A,
    0x0A, 0x04, 0x08, 0x60, 0x10, 0x12, 0x12, 0x04, 0x08, 0x10, 0x10, 0x58,
    0x12, 0x04, 0x08, 0x78, 0x10, 0x28, 0x12, 0x05, 0x08, 0xA8, 0x01, 0x10,
    0x38, 0x18, 0x1F, 0x1A, 0x00, 0x42, 0x53, 0x44, 0x46, 0x32, 0x01, 0x01,
    0x01, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x42, 0x5A, 0x68, 0x39, 0x17, 0x72, 0x45, 0x38, 0x50, 0x90, 0x00,
    0x00, 0x00, 0x00, 0x42, 0x5A, 0x68, 0x39, 0x17, 0x72, 0x45, 0x38, 0x50,
    0x90, 0x00, 0x00, 0x00, 0x00, 0x42, 0x5A, 0x68, 0x39, 0x17, 0x72, 0x45,
    0x38, 0x50, 0x90, 0x00, 0x00, 0x00, 0x00};

const Buffer kPatch1ToNoDeflate = {
    0x50, 0x55, 0x46, 0x31, 0x00, 0x00, 0x00, 0x2F, 0x08, 0x01, 0x12, 0x27,
    0x0A, 0x04, 0x08, 0x10, 0x10, 0x32, 0x0A, 0x04, 0x08, 0x50, 0x10, 0x0A,
    0x0A, 0x04, 0x08, 0x60, 0x10, 0x12, 0x12, 0x04, 0x08, 0x10, 0x10, 0x58,
    0x12, 0x04, 0x08, 0x78, 0x10, 0x28, 0x12, 0x05, 0x08, 0xA8, 0x01, 0x10,
    0x38, 0x18, 0x1F, 0x1A, 0x02, 0x18, 0x04, 0x42, 0x53, 0x44, 0x46, 0x32,
    0x01, 0x01, 0x01, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x42, 0x5A, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26, 0x53,
    0x59, 0xBA, 0x8D, 0x7F, 0x2D, 0x00, 0x00, 0x00, 0x40, 0x00, 0x44, 0x08,
    0x20, 0x00, 0x30, 0xCC, 0x09, 0x32, 0x54, 0x65, 0x38, 0xBB, 0x92, 0x29,
    0xC2, 0x84, 0x85, 0xD4, 0x6B, 0xF9, 0x68, 0x42, 0x5A, 0x68, 0x39, 0x17,
    0x72, 0x45, 0x38, 0x50, 0x90, 0x00, 0x00, 0x00, 0x00, 0x42, 0x5A, 0x68,
    0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0xE7, 0xAA, 0xF1, 0xFC, 0x00,
    0x00, 0x00, 0x70, 0x00, 0x00, 0x08, 0x01, 0x00, 0x20, 0x04, 0x20, 0x00,
    0x21, 0x9A, 0x68, 0x33, 0x4D, 0x13, 0x3C, 0x5D, 0xC9, 0x14, 0xE1, 0x42,
    0x43, 0x9E, 0xAB, 0xC7, 0xF0};

}  // namespace

void TestPatching(const Buffer& src_buf,
                  const Buffer& dst_buf,
                  const vector<BitExtent>& src_deflates,
                  const vector<BitExtent>& dst_deflates,
                  const Buffer patch) {
  Buffer patch_out;
  string patch_path;
  ASSERT_TRUE(MakeTempFile(&patch_path, nullptr));
  ScopedPathUnlinker scoped_unlinker(patch_path);
  ASSERT_TRUE(PuffDiff(src_buf, dst_buf, src_deflates, dst_deflates,
                       {bsdiff::CompressorType::kBZ2}, patch_path, &patch_out));

#if PRINT_SAMPLE
  PrintArray("kPatchXXXXX", patch_out);
#endif

  EXPECT_EQ(patch_out, patch);

  auto src_stream = MemoryStream::CreateForRead(src_buf);
  Buffer dst_buf_out(dst_buf.size());
  auto dst_stream = MemoryStream::CreateForWrite(&dst_buf_out);
  ASSERT_TRUE(PuffPatch(std::move(src_stream), std::move(dst_stream),
                        patch.data(), patch.size()));
  EXPECT_EQ(dst_buf_out, dst_buf);
}

TEST(PatchingTest, Patching1To2Test) {
  TestPatching(kDeflatesSample1, kDeflatesSample2,
               kSubblockDeflateExtentsSample1, kSubblockDeflateExtentsSample2,
               kPatch1To2);
}

TEST(PatchingTest, Patching2To1Test) {
  TestPatching(kDeflatesSample2, kDeflatesSample1,
               kSubblockDeflateExtentsSample2, kSubblockDeflateExtentsSample1,
               kPatch2To1);
}

TEST(PatchingTest, Patching1ToEmptyTest) {
  TestPatching(kDeflatesSample1, {}, kSubblockDeflateExtentsSample1, {},
               kPatch1ToEmpty);
}

TEST(PatchingTest, Patching1ToNoDeflateTest) {
  TestPatching(kDeflatesSample1, {11, 22, 33, 44},
               kSubblockDeflateExtentsSample1, {}, kPatch1ToNoDeflate);
}

// TODO(ahassani): add tests for:
//   TestPatchingEmptyTo2
//   TestPatchingNoDeflateTo2

// TODO(ahassani): Change tests data if you decided to compress the header of
// the patch.

}  // namespace puffin
