#pragma once

#ifndef S16MIN
#define S16MIN  -32768     
#endif 
#ifndef S16MAX
#define S16MAX  32767      
#endif
#ifndef S32MIN
#define S32MIN  -2147483648  
#endif
#ifndef S32MAX
#define S32MAX  2147483647  
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
