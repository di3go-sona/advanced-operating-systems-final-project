#define DEBUG_ENABLED

#ifdef DEBUG_ENABLED
#define _DEBUG(...) do{  printk( KERN_INFO "[ DEBUG ]" __VA_ARGS__ );} while( 0 )
#define _ERROR(...) do{  printk( KERN_ERR  "[ ERROR ]" __VA_ARGS__ );} while( 0 )

#else
#define DEBUG(...) do{ } while ( 0 )
#define ERROR(...) do{ } while ( 0 )
#endif

#define DEBUG(...) do{  _DEBUG( " " __VA_ARGS__ ) ;} while( 0 )
#define ERROR(...) do{  _ERROR( " " __VA_ARGS__ ) ;} while( 0 )

#define GR_DEBUG(...) do{ _DEBUG("[ GROUP_ROOT ] " __VA_ARGS__);} while( 0 )
#define GR_ERROR(...) do{ _ERROR("[ GROUP_ROOT ] " __VA_ARGS__);} while( 0 )