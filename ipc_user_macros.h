// #define DEBUG_ENABLED 


#ifdef DEBUG_ENABLED
#define _DEBUG(...) do{ fprintf(stderr, "\x1b[30;1m[ DEBUG ][ pid %d ] ", getpid()); fprintf( stderr, __VA_ARGS__ ); fprintf(stderr, "\x1b[0m\n");} while( 0 )
#define _ERR(...) do{ fprintf(stderr, "\x1b[31;1m[ ERR ][ pid %d ] ", getpid()); fprintf( stderr, __VA_ARGS__ ); fprintf(stderr, "\x1b[0m\n");} while( 0 )
#define _WARN(...) do{ fprintf(stderr, "\x1b[33;1m[ WARN ][ pid %d ] ", getpid()); fprintf( stderr, __VA_ARGS__ ); fprintf(stderr, "\x1b[0m\n");} while( 0 )

#else
#define _DEBUG(...) do{ } while ( 0 )
#define _ERR(...) do{ } while ( 0 )
#define _WARN(...) do{ } while ( 0 )
#endif


#define DEBUG(...) do{ _DEBUG( __VA_ARGS__ );} while( 0 )
#define ERR(...) do{ _ERR( __VA_ARGS__ );} while( 0 )
#define WARN(...) do{ _WARN( __VA_ARGS__ );} while( 0 )


#define TEST_BEGIN(...) do{ printf( "[ TEST_BEGIN ] "); printf(  __VA_ARGS__ ); printf("\n");} while( 0 )
#define TEST_END(...) do{ printf( "[ TEST_END ]\n\n"); } while( 0 )