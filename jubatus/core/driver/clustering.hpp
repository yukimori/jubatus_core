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

#ifndef JUBATUS_CORE_DRIVER_CLUSTERING_HPP_
#define JUBATUS_CORE_DRIVER_CLUSTERING_HPP_

#include <utility>
#include <vector>
#include "jubatus/util/lang/shared_ptr.h"
#include "../clustering/types.hpp"
#include "../framework/mixable.hpp"
#include "../fv_converter/datum.hpp"
#include "../fv_converter/mixable_weight_manager.hpp"
#include "driver.hpp"

namespace jubatus {
namespace core {
namespace fv_converter {
class datum;
class datum_to_fv_converter;
}  // namespace fv_converter
namespace clustering {
class clustering;
}  // namespace clustering
namespace driver {

class clustering : public driver_base {
 public:
  clustering(
      jubatus::util::lang::shared_ptr<core::clustering::clustering>
          clustering_method,
      jubatus::util::lang::shared_ptr<fv_converter::datum_to_fv_converter>
          converter);
  virtual ~clustering();

  void push(const std::vector<core::clustering::indexed_point>& points);

  fv_converter::datum get_nearest_center(
      const fv_converter::datum& point) const;
  core::clustering::cluster_unit get_nearest_members(
    const fv_converter::datum& point) const;
  core::clustering::index_cluster_unit get_nearest_members_light(
      const fv_converter::datum& point) const;

  std::vector<fv_converter::datum> get_k_center() const;
  core::clustering::cluster_set get_core_members() const;
  core::clustering::index_cluster_set get_core_members_light() const;

  size_t get_revision() const;
  void pack(framework::packer& pk) const;
  void unpack(msgpack::object o);

  // for test only
  void do_clustering();

  void clear();

 private:
  common::sfv_t to_sfv(const fv_converter::datum& dat);
  common::sfv_t to_sfv_const(const fv_converter::datum& dat) const;
  fv_converter::datum to_datum(const common::sfv_t& src) const;
  core::clustering::weighted_point to_weighted_point(
      const fv_converter::datum& src);
  core::clustering::weighted_point to_weighted_indexed_point(
      const core::clustering::indexed_point& src);
  std::pair<double, fv_converter::datum>
  to_weighted_datum(const core::clustering::weighted_point& src) const;
  std::pair<double, std::string>
  to_weighted_index(const core::clustering::weighted_point& src) const;
  std::vector<fv_converter::datum> to_datum_vector(
      const std::vector<common::sfv_t>& src) const;
  std::vector<core::clustering::weighted_point> to_weighted_point_vector(
      const std::vector<fv_converter::datum>& src);
  std::vector<core::clustering::weighted_point> to_weighted_indexed_point_vector(
      const std::vector<core::clustering::indexed_point>& src);
  core::clustering::cluster_unit to_weighted_datum_vector(
      const std::vector<core::clustering::weighted_point>& src) const;
  core::clustering::index_cluster_unit to_weighted_index_vector(
      const std::vector<core::clustering::weighted_point>& src) const;

  jubatus::util::lang::shared_ptr<fv_converter::datum_to_fv_converter>
    converter_;
  jubatus::util::lang::shared_ptr<core::clustering::clustering> clustering_;
  fv_converter::mixable_weight_manager wm_;
};

}  // namespace driver
}  // namespace core
}  // namespace jubatus

#endif  // JUBATUS_CORE_DRIVER_CLUSTERING_HPP_
