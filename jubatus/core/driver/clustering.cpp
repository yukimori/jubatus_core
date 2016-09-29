// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2013 Preferred Networks and Nippon Telegraph and Telephone Corporation.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "clustering.hpp"

#include <algorithm>
#include <utility>
#include <vector>
#include "jubatus/util/lang/bind.h"
#include "../clustering/clustering.hpp"
#include "../fv_converter/datum_to_fv_converter.hpp"
#include "../common/vector_util.hpp"
#include "../fv_converter/revert.hpp"
#include "../fv_converter/weight_manager.hpp"
#include "../fv_converter/mixable_weight_manager.hpp"

namespace jubatus {
namespace core {
namespace driver {

using std::vector;
using std::pair;
using jubatus::util::lang::shared_ptr;
using fv_converter::datum;
using fv_converter::mixable_weight_manager;
using fv_converter::weight_manager;

clustering::clustering(
    shared_ptr<core::clustering::clustering> clustering_method,
    shared_ptr<fv_converter::datum_to_fv_converter> converter)
    : converter_(converter),
      clustering_(clustering_method),
      wm_(mixable_weight_manager::model_ptr(new weight_manager)) {
  register_mixable(clustering_->get_mixable());
  register_mixable(&wm_);

  converter_->set_weight_manager(wm_.get_model());
}

clustering::~clustering() {
}

void clustering::push(const vector<core::clustering::indexed_point>& points) {
  clustering_->push(to_weighted_indexed_point_vector(points));
}

datum clustering::get_nearest_center(
    const datum& point) const {
  return to_datum(
      clustering_->get_nearest_center(to_sfv_const(point)));
}

core::clustering::cluster_unit
    clustering::get_nearest_members(const datum& point) const {
  return to_weighted_datum_vector(
      clustering_->get_nearest_members(to_sfv_const(point)));
}

core::clustering::index_cluster_unit
clustering::get_nearest_members_light(const datum& point) const {
  return to_weighted_index_vector(
      clustering_->get_nearest_members(to_sfv_const(point)));
}

vector<datum> clustering::get_k_center() const {
  return to_datum_vector(clustering_->get_k_center());
}

core::clustering::cluster_set
clustering::get_core_members() const {
  vector<vector<core::clustering::weighted_point> > src =
      clustering_->get_core_members();

  core::clustering::cluster_set ret;
  ret.reserve(src.size());
  std::transform(
      src.begin(),
      src.end(),
      std::back_inserter(ret),
      jubatus::util::lang::bind(
          &clustering::to_weighted_datum_vector,
          this, jubatus::util::lang::_1));

  return ret;
}

core::clustering::index_cluster_set
clustering::get_core_members_light() const {
  vector<vector<core::clustering::weighted_point> > src =
      clustering_->get_core_members();

  core::clustering::index_cluster_set ret;
  ret.reserve(src.size());
  std::transform(
      src.begin(),
      src.end(),
      std::back_inserter(ret),
      jubatus::util::lang::bind(
          &clustering::to_weighted_index_vector,
          this, jubatus::util::lang::_1));

  return ret;
}

size_t clustering::get_revision() const {
  return clustering_->get_revision();
}

void clustering::do_clustering() {
  clustering_->do_clustering();
}

// private

common::sfv_t clustering::to_sfv(const datum& dat) {
  common::sfv_t ret;
  converter_->convert_and_update_weight(dat, ret);
  common::sort_and_merge(ret);
  return ret;
}


common::sfv_t clustering::to_sfv_const(const datum& dat) const {
  common::sfv_t ret;
  converter_->convert(dat, ret);
  common::sort_and_merge(ret);
  return ret;
}

datum clustering::to_datum(const common::sfv_t& src) const {
  datum ret;
  fv_converter::revert_feature(src, ret);
  return ret;
}


pair<double, datum> clustering::to_weighted_datum(
  const core::clustering::weighted_point& src) const {
  return std::make_pair(src.weight, src.original);
}

pair<double, std::string> clustering::to_weighted_index(
    const core::clustering::weighted_point& src) const {
  return std::make_pair(src.weight, src.id);
}

core::clustering::weighted_point clustering::to_weighted_point(
    const datum& src) {
  core::clustering::weighted_point ret;
  ret.data = to_sfv(src);
  ret.weight = 1;
  ret.original = src;
  return ret;
}

core::clustering::weighted_point clustering::to_weighted_indexed_point(
    const core::clustering::indexed_point& src) {
  core::clustering::weighted_point ret;
  ret.id = src.id;
  ret.data = to_sfv(src.point);
  ret.weight = 1;
  ret.original = src.point;
  return ret;
}

vector<datum> clustering::to_datum_vector(
  const vector<common::sfv_t>& src) const {
  vector<datum> ret;
  ret.reserve(src.size());
  std::transform(
      src.begin(),
      src.end(),
      std::back_inserter(ret),
      jubatus::util::lang::bind(
          &clustering::to_datum,
          this, jubatus::util::lang::_1));
  return ret;
}

vector<core::clustering::weighted_point>
clustering::to_weighted_point_vector(
    const vector<datum>& src) {
  vector<core::clustering::weighted_point> ret;
  ret.reserve(src.size());
  std::transform(
      src.begin(),
      src.end(),
      std::back_inserter(ret),
      jubatus::util::lang::bind(
          &clustering::to_weighted_point,
          this, jubatus::util::lang::_1));
  return ret;
}

vector<core::clustering::weighted_point>
  clustering::to_weighted_indexed_point_vector(
      const vector<core::clustering::indexed_point>& src) {
  vector<core::clustering::weighted_point> ret;
  ret.reserve(src.size());
  std::transform(
      src.begin(),
      src.end(),
      std::back_inserter(ret),
      jubatus::util::lang::bind(
          &clustering::to_weighted_indexed_point,
          this, jubatus::util::lang::_1));
  return ret;
}

core::clustering::cluster_unit
clustering::to_weighted_datum_vector(
    const vector<core::clustering::weighted_point>& src) const {
  core::clustering::cluster_unit ret;
  ret.reserve(src.size());
  std::transform(
      src.begin(),
      src.end(),
      std::back_inserter(ret),
      jubatus::util::lang::bind(
          &clustering::to_weighted_datum,
          this, jubatus::util::lang::_1));
  return ret;
}

core::clustering::index_cluster_unit
clustering::to_weighted_index_vector(
    const vector<core::clustering::weighted_point>& src) const {
  core::clustering::index_cluster_unit ret;
  ret.reserve(src.size());
  std::transform(
      src.begin(),
      src.end(),
      std::back_inserter(ret),
      jubatus::util::lang::bind(
          &clustering::to_weighted_index,
          this, jubatus::util::lang::_1));

  return ret;
}

void clustering::pack(framework::packer& pk) const {
  pk.pack_array(2);
  clustering_->pack(pk);
  wm_.get_model()->pack(pk);
}

void clustering::unpack(msgpack::object o) {
  if (o.type != msgpack::type::ARRAY || o.via.array.size != 2) {
    throw msgpack::type_error();
  }

  // clear
  clustering_->clear();
  converter_->clear_weights();

  // load
  clustering_->unpack(o.via.array.ptr[0]);
  wm_.get_model()->unpack(o.via.array.ptr[1]);
}

void clustering::clear() {
  clustering_->clear();
  converter_->clear_weights();
}

}  // namespace driver
}  // namespace core
}  // namespace jubatus
