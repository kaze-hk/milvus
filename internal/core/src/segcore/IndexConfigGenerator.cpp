// Copyright (C) 2019-2020 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under the License

#include "IndexConfigGenerator.h"
#include "log/Log.h"

namespace milvus::segcore {
VecIndexConfig::VecIndexConfig(const int64_t max_index_row_cout,
                               const FieldIndexMeta& index_meta_,
                               const SegcoreConfig& config,
                               const SegmentType& segment_type)
    : max_index_row_count_(max_index_row_cout), config_(config) {
    origin_index_type_ = index_meta_.GetIndexType();
    metric_type_ = index_meta_.GeMetricType();

    index_type_ = support_index_types.at(segment_type);
    build_params_[knowhere::meta::METRIC_TYPE] = metric_type_;
    build_params_[knowhere::indexparam::NLIST] =
        std::to_string(config_.get_nlist());
    build_params_[knowhere::indexparam::SSIZE] = std::to_string(
        std::max((int)(config_.get_chunk_rows() / config_.get_nlist()), 48));
    search_params_[knowhere::indexparam::NPROBE] =
        std::to_string(config_.get_nprobe());
    LOG_SEGCORE_INFO_ << " VecIndexConfig: "
                      << " origin_index_type_:" << origin_index_type_
                      << " index_type_: " << index_type_
                      << " metric_type_: " << metric_type_;
}

int64_t
VecIndexConfig::GetBuildThreshold() const noexcept {
    assert(VecIndexConfig::index_build_ratio.count(index_type_));
    auto ratio = VecIndexConfig::index_build_ratio.at(index_type_);
    assert(ratio >= 0.0 && ratio < 1.0);
    return std::max(int64_t(max_index_row_count_ * ratio),
                    config_.get_nlist() * 39);
}

knowhere::IndexType
VecIndexConfig::GetIndexType() noexcept {
    return index_type_;
}

knowhere::MetricType
VecIndexConfig::GetMetricType() noexcept {
    return metric_type_;
}

knowhere::Json
VecIndexConfig::GetBuildBaseParams() {
    return build_params_;
}

SearchInfo
VecIndexConfig::GetSearchConf(const SearchInfo& searchInfo) {
    SearchInfo searchParam(searchInfo);
    searchParam.metric_type_ = metric_type_;
    searchParam.search_params_ = search_params_;
    for (auto& key : maintain_params) {
        if (searchInfo.search_params_.contains(key)) {
            searchParam.search_params_[key] = searchInfo.search_params_[key];
        }
    }
    return searchParam;
}

}  // namespace milvus::segcore