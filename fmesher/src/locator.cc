#include <cstddef>
#include <cstring>
#include <set>
#include <map>
#include <sstream>
#include <cmath>

#include "locator.hh"

#define WHEREAMI __FILE__ << "(" << __LINE__ << ")\t"

#define LOG_(msg) std::cout << WHEREAMI << msg;
#ifdef DEBUG
#define LOG(msg) LOG_(msg)
#else
#define LOG(msg)
#endif


using std::cout;
using std::endl;

namespace fmesh {

  TriangleLocator::TriangleLocator(const Mesh* mesh,
				   const std::vector<int>& dimensions,
				   bool use_interval_tree) : 
    mesh_(mesh),
    dim_(dimensions),
    bbox_(),
    bbox_locator_(dimensions.size(),use_interval_tree)
  {
    bbox_.resize(dim_.size());
    if (mesh_) {
      for (int i=0; i<dim_.size(); ++i) {
	bbox_[i].resize(mesh_->nT());
      }
      
      /* Build boxes: */
      int d;
      Point mini;
      Point maxi;
      std::pair<double, double> range;
      for (int t=0; t<mesh_->nT(); ++t) {
	mesh_->triangleBoundingBox(t,mini,maxi);
	for (int di=0; di<dim_.size(); ++di) {
	  d = dim_[di];
	  range.first = mini[d];
	  range.second = maxi[d];
	  bbox_[di][t] = range;
	}
      }
    }
    
    bbox_locator_.init(bbox_.begin());
  }

  TriangleLocator::~TriangleLocator()
  {
    /* Nothing to do. */
  }


  int TriangleLocator::locate(const Point& s) const {
    std::vector<double> loc(dim_.size());
    for (int di=0; di<dim_.size(); ++di) {
      loc[di] = s[dim_[di]];
    }
    Dart d;
    for (typename bbox_locator_type::search_iterator si =
	   bbox_locator_.search_begin(loc);
	 !si.is_null();
	 ++si) {
      d = mesh_->locate_point(Dart(*mesh_,(*si)),s);
      if (!d.isnull())
	return (d.t());
    }
    return -1;
  }

  std::ostream& TriangleLocator::print(std::ostream& output)
  {
    return bbox_locator_.print(output);
  }
 
  std::ostream& operator<<(std::ostream& output, TriangleLocator& locator)
  {
    return locator.print(output);
  }


} /* namespace fmesh */
