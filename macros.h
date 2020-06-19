#pragma once

#ifndef S16MIN
#define S16MIN  0x8000      
#endif 
#ifndef S16MAX
#define S16MAX  0x7fff      
#endif
#ifndef S32MIN
#define S32MIN  0x80000000  
#endif
#ifndef S32MAX
#define S32MAX  0x7fffffff  
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)       \
   if(x != NULL)             \
   {                         \
      delete x;              \
      x = NULL;              \
   }
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
   if(x != NULL)             \
   {                         \
      delete[] x;            \
      x = NULL;              \
   }
#endif
