// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_TABLE_FORMAT_H_
#define STORAGE_LEVELDB_TABLE_FORMAT_H_

#include <string>
#include <stdint.h>
#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "leveldb/table_builder.h"

namespace leveldb {

class Block;
class RandomAccessFile;
struct ReadOptions;

// BlockHandle is a pointer to the extent of a file that stores a data
// block or a meta block.
class BlockHandle {
 public:
  BlockHandle();

  // The offset of the block in the file.
  uint64_t offset() const { return offset_; }
  void set_offset(uint64_t offset) { offset_ = offset; }

  // The size of the stored block
  uint64_t size() const { return size_; }
  void set_size(uint64_t size) { size_ = size; }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* input);

  // BlockHandle长度为2个varint64，因此最大长度为20
  // Maximum encoding length of a BlockHandle
  enum { kMaxEncodedLength = 10 + 10 };

 private:
  uint64_t offset_; // 所指block的offset
  uint64_t size_;   // 所指block的size
};

// Footer encapsulates the fixed information stored at the tail
// end of every table file.
class Footer {
 public:
  Footer() { }

  // The block handle for the metaindex block of the table
  const BlockHandle& metaindex_handle() const { return metaindex_handle_; }
  void set_metaindex_handle(const BlockHandle& h) { metaindex_handle_ = h; }

  // The block handle for the index block of the table
  const BlockHandle& index_handle() const {
    return index_handle_;
  }
  void set_index_handle(const BlockHandle& h) {
    index_handle_ = h;
  }

  // 将Footer序列化后append在dst上
  void EncodeTo(std::string* dst) const;
  // 从input的data里反序列化出Footer
  Status DecodeFrom(Slice* input);

  // Footer 长度固定为2个BlockHandle长度和一个魔数长度
  //
  // Encoded length of a Footer.  Note that the serialization of a
  // Footer will always occupy exactly this many bytes.  It consists
  // of two block handles and a magic number.
  enum {
    kEncodedLength = 2*BlockHandle::kMaxEncodedLength + 8
  };

 private:
  BlockHandle metaindex_handle_; // meta index block的起始位置和大小，具体参见SST文件格式
  BlockHandle index_handle_;     // index block的起始地址和大小
};

// kTableMagicNumber was picked by running
//    echo http://code.google.com/p/leveldb/ | sha1sum
// and taking the leading 64 bits.
static const uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;

// 1-byte type + 32-bit crc
static const size_t kBlockTrailerSize = 5;

struct BlockContents {
  Slice data;           // Actual contents of data 只有block data，不包含type crc32
  bool cachable;        // True iff data can be cached
  bool heap_allocated;  // True iff caller should delete[] data.data()
};

// 根据handle的标识，读取一个block
// Read the block identified by "handle" from "file".  On failure
// return non-OK.  On success fill *result and return OK.
extern Status ReadBlock(RandomAccessFile* file,
                        const ReadOptions& options,
                        const BlockHandle& handle,
                        BlockContents* result);

// Implementation details follow.  Clients should ignore,

inline BlockHandle::BlockHandle()
    : offset_(~static_cast<uint64_t>(0)),
      size_(~static_cast<uint64_t>(0)) {
}

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_FORMAT_H_
