#include "stdafx.h"
#include <string_view>
#include <boost/preprocessor.hpp>
#include "glsl_loader.h"
#include <glsl/glsl.h>

namespace glsl {

#define SHADER_ENUM_TYPE_vs GL_VERTEX_SHADER
#define SHADER_ENUM_TYPE_frag GL_FRAGMENT_SHADER
#define GET_SHADER_ENUM_TYPE(ext) BOOST_PP_CAT(SHADER_ENUM_TYPE_, ext)


#define GET_NAME(Name, type) BOOST_PP_CAT(Name, BOOST_PP_CAT(_, type))
#define GET_LEN(Name, type) BOOST_PP_CAT(Name, BOOST_PP_CAT(BOOST_PP_CAT(_, type), _len))
#define GET_SHADER_SRC(Name, type)                                                                                     \
    { reinterpret_cast<const GLchar*>(GET_NAME(Name, type)), GET_LEN(Name, type) }

#define BIND_SHADER_TYPE(r, Name, type)                                                                                \
    template<>                                                                                                         \
    const shader_src Name<GET_SHADER_ENUM_TYPE(type)>::src GET_SHADER_SRC(Name, type);

#define LOAD_SHADER(Name, ...) BOOST_PP_SEQ_FOR_EACH(BIND_SHADER_TYPE, Name, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
}  // namespace glsl

namespace glsl {

    LOAD_SHADER(shader, vs, frag);

}
