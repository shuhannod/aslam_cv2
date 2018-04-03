#include "aslam/cameras/distortion.h"

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

DEFINE_double(acv_inv_distortion_tolerance, 1e-8, "Convergence tolerance for iterated"
              "inverse distortion functions.");

namespace aslam {
std::ostream& operator<<(std::ostream& out, const Distortion& distortion) {
  distortion.printParameters(out, std::string(""));
  return out;
}

Distortion::Distortion(const Eigen::VectorXd& dist_coeffs,
                       Type distortion_type, const unsigned image_width,
                       const unsigned& image_height)
    : distortion_coefficients_(dist_coeffs),
      distortion_type_(distortion_type),
      image_width_(image_width),
      image_height_(image_height) {}

bool Distortion::operator==(const Distortion& rhs) const {
  //check for same distortion type
  if (typeid(*this) != typeid(rhs))
    return false;

  if (distortion_coefficients_ != rhs.distortion_coefficients_)
    return false;

  return true;
}

void Distortion::calculateDistortionMap() {
  CHECK_GT(image_width_, 0);
  CHECK_GT(image_height_, 0);
  distortion_map_(image_width_, image_height_);
  for (size_t i = 0; i < image_width_, i++) {
    for (size_t j = 0; j < image_height_; j++) {
      Eigen::Vector2d point(i, j);
      undistort(point);
      distortion_map_.set(i, j, point);
    }
  }
  distortion_map_.calculated();
}


void Distortion::distort(Eigen::Vector2d* point) const {
  CHECK_NOTNULL(point);
  distortUsingExternalCoefficients(nullptr, point, nullptr);
}

void Distortion::distort(const Eigen::Vector2d& point,
                         Eigen::Vector2d* out_point) const {
  CHECK_NOTNULL(out_point);
  *out_point = point;
  distortUsingExternalCoefficients(nullptr, out_point, nullptr);
}

void Distortion::distort(
    Eigen::Vector2d* point,
    Eigen::Matrix2d* out_jacobian) const {
  CHECK_NOTNULL(point);
  CHECK_NOTNULL(out_jacobian);
  distortUsingExternalCoefficients(nullptr, point, out_jacobian);
}

void Distortion::undistort(Eigen::Vector2d* point) const {
  CHECK_NOTNULL(point);
  undistortUsingExternalCoefficients(distortion_coefficients_, point);
}

void Distortion::undistort(const Eigen::Vector2d& point,
                           Eigen::Vector2d* out_point) const {
  CHECK_NOTNULL(out_point);
  *out_point = point;
  undistortUsingExternalCoefficients(distortion_coefficients_, out_point);
}

void Distortion::undistortUsingMap(Eigen::Vector2d* point) const {
  CHECK_NOTNULL(point);
  CHECK_LE((*point)(0), rows_);
  CHECK_LE((*point)(1), cols_);
  if (!distortion_map_.available()) {
    VLOG(2) << "Calculate distortion map."
  }
  CHECK(distortion_map_.available());
  distortion_map_.get((*point)(0), (*point)(1), point);
}

void Distortion::undistortusingMap(const Eigen::Vector2d& point,
                           Eigen::Vector2d* out_point) const {
  CHECK_NOTNULL(out_point);
  *out_point = point;
  undistortUsingMap(out_point);
}

void Distortion::setParameters(const Eigen::VectorXd& dist_coeffs) {
  CHECK(distortionParametersValid(dist_coeffs)) << "Distortion parameters invalid!";
  distortion_coefficients_ = dist_coeffs;
}

}  // namespace aslam
