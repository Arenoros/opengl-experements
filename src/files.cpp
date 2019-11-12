#include "stdafx.h"
#include "api.h"

void test() {
    File file("opengl_test.exe", "rb");
    
    uint8_t buf[4*1024];
    while(file.read(buf, sizeof(buf))) {
        
    }

}
