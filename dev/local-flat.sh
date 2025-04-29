mkdir -p flat/boost/openmethod
python3 dev/flatten.py \
  flat/boost/openmethod.hpp \
  include/boost/openmethod.hpp \
  include/boost/openmethod/unique_ptr.hpp \
  include/boost/openmethod/shared_ptr.hpp \
  include/boost/openmethod/compiler.hpp
python3 dev/flatten.py \
  flat/boost/openmethod/policies.hpp \
  include/boost/openmethod/policies.hpp
