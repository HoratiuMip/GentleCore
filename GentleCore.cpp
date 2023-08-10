#pragma region Details
/*
    [ GENERAL ]
        Engine:      GentleCore

        Version:     Golf 1 | 25 May 2023

        Hackerman:   Mipsan

        C++:         2020's standard, ISO GCC extended

        OSs:         Windows

    [ PRE-DEFINES ]
        GTL_ECHO --- logs stuff.
        GTL_ONE_SURFACE --- enables quicker event routing.

    [ GCC FLAGS ]
        -static-libgcc
        -static-libc++

    [ LINKERS ]
        -lwinmm
        -lwindowscodecs
        -lole32
        -ld2d1
        -lComdlg32

*/
#pragma endregion Details



#pragma region Includes

    #include <iostream>
    #include <fstream>
    #include <string>
    #include <string_view>
    #include <stack>
    #include <queue>
    #include <vector>
    #include <array>
    #include <list>
    #include <forward_list>
    #include <map>
    #include <deque>
    #include <algorithm>
    #include <utility>
    #include <memory>
    #include <condition_variable>
    #include <cmath>
    #include <optional>
    #include <variant>
    #include <functional>
    #include <concepts>
    #include <type_traits>
    #include <chrono>
    #include <thread>
    #include <future>
    #include <semaphore>
    #include <tuple>
    #include <bitset>
    #include <atomic>

    #include <windows.h>
    #include <wincodec.h>
    #include <d2d1.h>

#pragma endregion Includes



#pragma region Defines



    #define PI 3.141592653



    #define _ENGINE_NAMESPACE Gtl

    #define _ENGINE_CLASS_NAME( name ) "Gtl :: " name


    #if defined( GTL_ECHO )
        #define _ENGINE_ECHO
    #endif

    #if defined( GTL_ONE_SURFACE )
        #define _ENGINE_ONE_SURFACE
    #endif



#pragma endregion Defines



namespace _ENGINE_NAMESPACE {



#pragma region Quintessentials

    typedef   void*   UID;



    enum Word {
        VAL, REF, IDX
    };



    class Vec2;
    class Ray2;
    class Clust2;

    class Surface;
    class Renderer;
    class Chroma;
    class Solid_brush;
    class Linear_brush;
    class Radial_brush;
    class Sprite;

    class Audio;
    class Sound;



    template< template< typename T > typename P, typename T, typename I >
    class _Smart_ptr_extended : public P< T > {
    public:
        using P< T > :: P;

        using P< T > :: operator =;

    public:
        inline static constexpr bool   is_array   = std :: is_array_v< T >;

    public:
        typedef   std :: conditional_t< is_array, std :: decay_t< T >, T* >   T_ptr;

        typedef   std :: remove_pointer_t< T_ptr >&   T_ref;

    public:
        I& operator = ( T_ptr ptr ) {
            this -> reset( ptr );

            return static_cast< I& >( *this );
        }
    
    public:
        operator T_ptr () {
            return this -> get();
        }

        template< bool isnt_array = !is_array >
        operator std :: enable_if_t< isnt_array, T_ref > () {
            return *this -> get();
        }

    public:
        T_ptr operator + ( ptrdiff_t n ) const {
            return this -> get() + n;
        }

        T_ptr operator - ( ptrdiff_t n ) const {
            return this -> get() - n;
        }

    };


    template< typename T >
    class Unique : public _Smart_ptr_extended< std :: unique_ptr, T, Unique< T > > {
    public:
        typedef   _Smart_ptr_extended< std :: unique_ptr, T, Unique< T > >   _Base;

        using _Base :: _Base;

        using _Base :: operator =;

    };


    template< typename T >
    class Shared : public _Smart_ptr_extended< std :: shared_ptr, T, Shared< T > > {
    public:
        typedef   _Smart_ptr_extended< std :: shared_ptr, T, Shared< T > >   _Base;

        using _Base :: _Base;

        using _Base :: operator =;

    };



    template< typename T >
    using Ref = T&;

    template< typename T >
    using Ptr = T*;



    template< typename T >
    concept Is_brush = ( 
        std :: is_same_v< Solid_brush, T >
        ||
        std :: is_same_v< Linear_brush, T >
        ||
        std :: is_same_v< Radial_brush, T >
    );

#pragma endregion Quintessentials



#pragma region Primitives

    class Rendable {
    public:
        virtual void render( Ref< Renderer >, Vec2 vec ) const = 0;

    };



    template< typename I >
    class Has_op_ptr {
    public:
        operator I* () {
            return reinterpret_cast< I* >( this );
        }

    };

#pragma endregion Primitives



#pragma region Echos


    #if defined( _ENGINE_ECHO )
        #define _ECHO_LITERAL( op, code ) \
            std :: string_view operator "" op ( const char* str, size_t unused ) { \
                SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), code ); \
                return str; \
            }

        _ECHO_LITERAL( _normal, 15 )
        _ECHO_LITERAL( _highlight, 8 )
        _ECHO_LITERAL( _special, 9 )
    #endif


    class Echo {
    public:
        Echo() = default;

        Echo( const Echo& other )
            : _level{ other._level + 1 }
        {}

        ~Echo() {
            if( _level == 0 )
                std :: cout << '\n';
        }

    private:
        size_t   _level   = 0;

    public:
        enum Type {
            FAULT = 12, WARNING = 14, OK = 10, PENDING = 9, HEADSUP = 13,
        };

    public: 
        const Echo& operator () ( 
            auto*              sender,
            Type               type,
            std :: string_view message 
        ) const {
            #if defined( _ENGINE_ECHO )
                std :: cout << " [ "_normal;

                _type_to( type );

                std :: cout << _type_name( type ) << " ] "_normal << _type_fill( type ) << "   "_special;

                for( size_t l = 0; l < _level; ++l )
                    std :: cout << "-";

                std :: cout
                << "From "_normal 
                << "[ "
                << ""_highlight 
                << std :: remove_pointer_t< decltype( sender ) > :: name 
                << " ][ "_normal
                << ""_highlight
                << static_cast< void* >( sender )
                << " ]  "_normal
                << "->  "_special
                << ""_normal
                << message
                << "\n"_normal;
            #endif

            return *this;
        }

    private:
        void _type_to( Type type ) const {
            SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), type );
        }

        std :: string_view _type_name( Type type ) const {
            switch( type ) {
                case FAULT:   return "FAULT";
                case WARNING: return "WARNING";
                case OK:      return "OK";
                case PENDING: return "PENDING";
                case HEADSUP: return "HEADSUP";
            }

            return "UNKNOWN";
        }

        std :: string_view _type_fill( Type type ) const {
            switch( type ) {
                case FAULT:   return "  ";
                case WARNING: return "";
                case OK:      return "     ";
                case PENDING: return "";
                case HEADSUP: return "";
            }

            return "";
        }

    };



#pragma endregion Echos



#pragma region Utility



    template< typename T >
    struct Coord {
        Coord() = default;

        Coord( T x, T y ) 
            : x( x ), y( y ) 
        {}

        template< typename T_other >
        Coord( const Coord< T_other >& other ) 
            : x( static_cast< T >( other.x ) ), y( static_cast< T >( other.y ) ) 
        {}


        T   x   = {};
        T   y   = {};


        template< bool is_float = std :: is_same_v< float, T > >
        operator std :: enable_if_t< is_float, const D2D1_POINT_2F& > () const {
            return *reinterpret_cast< const D2D1_POINT_2F* >( this );
        }

        template< bool is_float = std :: is_same_v< float, T > >
        operator std :: enable_if_t< is_float, D2D1_POINT_2F& > () {
            return *reinterpret_cast< D2D1_POINT_2F* >( this );
        }
        
    };

    template< typename T >
    struct Size {
        Size() = default;

        Size( T width, T height )
            : width( width ), height( height ) 
        {}

        template< typename T_other >
        Size( const Size< T_other >& other )
            : width( static_cast< T >( other.width ) ), height( static_cast< T >( other.height ) ) 
        {}

        T   width    = {};
        T   height   = {};

    };



    class File {
        public:
            static std :: string dir_of( std :: string_view path ) {
                return path.substr( 0, path.find_last_of( "/\\" ) ).data();
            }

            static std :: string name_of( std :: string_view path ) {
                return path.substr( path.find_last_of( "/\\" ) + 1, path.size() - 1 ).data();
            }
        
        public:
            static size_t size( std :: string_view path ) {
                std :: ifstream file( path.data(), std :: ios_base :: binary );

                return size( file );
            }

            static size_t size( std :: ifstream& file ) {
                file.seekg( 0, std :: ios_base :: end );

                size_t sz = file.tellg(); 

                file.seekg( 0, std :: ios_base :: beg );

                return sz;
            }

        public:
            static std :: string browse( std :: string_view title ) {
                char path[ MAX_PATH ];

                OPENFILENAME hf;

                std :: fill_n( path, sizeof( path ), 0 );
                std :: fill_n( reinterpret_cast< char* >( &hf ), sizeof( hf ), 0 );

                hf.lStructSize  = sizeof( hf );
                hf.hwndOwner    = GetFocus();
                hf.lpstrFile    = path;
                hf.nMaxFile     = MAX_PATH;
                hf.lpstrTitle   = title.data();
                hf.Flags        = OFN_EXPLORER | OFN_NOCHANGEDIR;

                GetOpenFileName( &hf );

                return path;
            }

            static std :: string save( std :: string_view title ) {
                char path[ MAX_PATH ];

                OPENFILENAME hf;

                std :: fill_n( path, sizeof( path ), 0 );
                std :: fill_n( reinterpret_cast< char* >( &hf ), sizeof( hf ), 0 );

                hf.lStructSize  = sizeof( hf );
                hf.hwndOwner    = GetFocus();
                hf.lpstrFile    = path;
                hf.nMaxFile     = MAX_PATH;
                hf.lpstrTitle   = title.data();
                hf.Flags        = OFN_EXPLORER | OFN_NOCHANGEDIR;

                GetSaveFileName( &hf );

                return path;
            }

        public:
            template< typename Itr >
            static std :: optional< ptrdiff_t > next_idx(
                std :: ifstream& file, std :: string& str,
                Itr begin, Itr end 
            ) {
                if( !( file >> str ) ) return {};

                return std :: distance(  
                    begin,                                                       
                    std :: find_if( begin, end, [ &str ] ( const decltype( *begin )& entry ) -> bool {
                        return str == entry;
                    } )
                );
            }

            template< typename Itr >
            static void auto_nav( 
                std :: ifstream& file,
                Itr begin, Itr end, 
                std :: function< void( ptrdiff_t, std :: string& ) > func  
            ) {
                std :: string str = {};

                for( 
                    auto idx = next_idx( file, str, begin, end ); 
                    idx.has_value(); 
                    idx = next_idx( file, str, begin, end ) 
                ) {
                    func( idx.value(), str );
                }
            }

        };



    class Bytes {
    public:
        enum Endianess {
            LITTLE, BIG
        };

    public:
        template< class T >
        static T as( char* array, size_t byte_count, Endianess end ) {
            char bytes[ sizeof( T ) ];

            const bool is_negative = 
                ( *reinterpret_cast< char* >( array + ( end == LITTLE ? byte_count - 1 : 0 ) ) ) >> 7
                && 
                std :: is_signed_v< T >;

            for( size_t n = byte_count; n < sizeof( T ); ++n )
                bytes[ n ] = is_negative ? -1 : 0;

            for( size_t n = 0; n < byte_count && n < sizeof( T ); ++n )
                bytes[ n ] = array[ end == LITTLE ? n : byte_count - n - 1 ];

            return *reinterpret_cast< T* >( &bytes );
        }
    };



    class Env {
    public:
        static int W() {
            static int value = ( [] () -> int {
                RECT rect;
                GetWindowRect( GetDesktopWindow(), &rect );

                return rect.right;
            } )();

            return value;
        }

        static int H_W() {
            return W() / 2;
        }

        static int T_W() {
            return W() / 3;
        }

        static int Q_W() {
            return W() / 4;
        }

        static int H() {
            static int value = ( [] () -> int {
                RECT rect;
                GetWindowRect( GetDesktopWindow(), &rect );

                return rect.bottom;
            } )();

            return value;
        }

        static int H_H() {
            return H() / 2;
        }

        static int T_H() {
            return H() / 3;
        }

        static int Q_H() {
            return H() / 4;
        }

        static float D() {
            static float value = std :: sqrt( W() * W() + H() * H() );

            return value;
        }

        static float H_D() {
            return D() / 2.0;
        }

        static float A() {
            return static_cast< float >( W() ) / H();
        }

        static std :: string_view dir() {
            static std :: string value = ( [] () -> std :: string {
                char path[ MAX_PATH ];

                GetModuleFileNameA( GetModuleHandle( NULL ), path, MAX_PATH );

                return File :: dir_of( path );
            } )();

            return value;
        }

        static std :: string_view process() {
            static std :: string value = ( [] () -> std :: string {
                char path[ MAX_PATH ];

                GetModuleFileNameA( GetModuleHandle( NULL ), path, MAX_PATH );

                return File :: name_of( path );
            } )();

            return value;
        }
    };



#pragma endregion Utility



#pragma region Syncs

    template< typename T >
    requires ( std :: is_arithmetic_v< T > )
    void wait_for( T duration ) {
        if constexpr( std :: is_floating_point_v< T > )
            std :: this_thread :: sleep_for( std :: chrono :: milliseconds( static_cast< int >( duration * 1000.0 ) ) );
        else
            std :: this_thread :: sleep_for( std :: chrono :: milliseconds( static_cast< int >( duration ) ) );
    }



    class Clock {
        public:
            Clock()
                : _create{ std :: chrono :: high_resolution_clock :: now() },
                  _last_lap{ std :: chrono :: high_resolution_clock :: now() }
            {}

        public:
            inline static constexpr double M[ 4 ] = { 1000.0, 1.0, 1.0 / 60.0, 1.0 / 3600.0 };

            enum Unit {
                MILLI = 0, SEC, MIN, HOUR
            };

        private:
            std :: chrono :: high_resolution_clock :: time_point   _create     = {};
            std :: chrono :: high_resolution_clock :: time_point   _last_lap   = {};

        public:
            double up_time( Unit unit = SEC ) const {
                using namespace std :: chrono;

                return duration< double >( high_resolution_clock :: now() - _create ).count() * M[ unit ];
            }

            double peek_lap( Unit unit = SEC ) const {
                using namespace std :: chrono;

                return duration< double >( high_resolution_clock :: now() - _last_lap ).count() * M[ unit ];
            }

            double lap( Unit unit = SEC ){
                using namespace std :: chrono;

                auto now = high_resolution_clock :: now();

                return duration< double >( now - std :: exchange( _last_lap, now ) ).count() * M[ unit ];
            }

        public:
            static auto UNIX() {
                return time( nullptr );
            }
    };



    template< typename T >
    class Controller {
    public:
        Controller() = default;


        Controller( const T& val )
            : _value( val ) 
        {}

        Controller( T&& val ) noexcept
            : _value( std :: move( val ) ) 
        {}
    

        Controller( const Controller< T >& other ) = delete;

        Controller( Controller< T >&& other ) = delete;


        ~Controller() {
            release();
        }
    
    private:
        typedef std :: tuple< 
                    Unique< std :: mutex >, 
                    Unique< std :: condition_variable >,
                    std :: function< bool( const T& ) >
                > Entry;

        enum _TUPLE_ACCESS_INDEX {
            _MTX = 0, _CND = 1, _OP = 2
        };

    private:
        mutable T              _value      = {};
        std :: mutex           _sync_mtx   = {}; 
        std :: list< Entry >   _entries    = {};

    public:
        operator typename std :: enable_if_t< std :: is_copy_constructible_v< T >, T > () const {
            return _value;
        }

    public:
        Controller& operate( std :: function< bool( T& ) > op ) {
            std :: unique_lock< std :: mutex > sync_lock( _sync_mtx );

            op( _value );

            for( Entry& entry : _entries )
                if( std :: get< _OP >( entry )( _value ) )
                    std :: get< _CND >( entry ) -> notify_all();

            return *this;
        }

        Controller& operator () ( std :: function< bool( T& ) > op ) {
            return operate( op );
        }

    public:
        Controller& wait_until( std :: function< bool( const T& ) > cnd ) {
            std :: unique_lock< std :: mutex > sync_lock( _sync_mtx );

            if( cnd( _value ) ) return *this;

            
            _entries.emplace_back( 
                std :: make_unique< std :: mutex >(), 
                new std :: condition_variable, 
                cnd 
            );

            auto entry = _entries.rbegin();

            sync_lock.unlock();


            std :: unique_lock< std :: mutex > lock( *std :: get< _MTX >( *entry ) );

            std :: get< _CND >( *entry ) -> wait( lock );

            lock.unlock();
            lock.release();

            _entries.erase( entry );

            return *this;
        }

    public:
        Controller& release() {
            for( Entry& entry : _entries )
                std :: get< _CND >( entry ) -> notify_all();

            _entries.clear();
        }

    };

#pragma endregion Syncs



#pragma region Space



    enum Heading {
        NORTH = 0, EAST, SOUTH, WEST
    };

    enum System {
        LOCAL = 0, GLOBAL
    };



    class Deg {
    public:
        static double pull( double theta ) {
            return theta * ( 180.0 / PI );
        }

        static void push( double& theta ) {
            theta *= ( 180.0 / PI );
        }
    };

    class Rad {
    public:
        static double pull( double theta ) {
            return theta * ( PI / 180.0 );
        }

        static void push( double& theta ) {
            theta *= ( PI / 180.0 );
        }
    };



    #pragma region D2



        class Ray2;
        class Clust2;



        class Vec2 {
        public:
            Vec2() = default;

            Vec2( double x, double y ) 
                : x( x ), y( y ) 
            {}

            Vec2( double x )
                : Vec2{ x, x }
            {}

            Vec2( const Vec2& other )
                : x( other.x ), y( other.y ) 
            {}

        public:
            double   x   = 0.0;
            double   y   = 0.0;

        public:
            double dot( const Vec2& other ) const {
                return x * other.x + y * other.y;
            }
            
        public:
            double mag_sq() const {
                return x * x + y * y;
            }

            double mag() const {
                return std :: sqrt( mag_sq() );
            }

            double angel() const {
                return Deg :: pull( std :: atan2( y, x ) );
            }

        public:
            double dist_sq_to( const Vec2& other ) const {
                return ( other.x - x ) * ( other.x - x ) + ( other.y - y ) * ( other.y - y );
            }

            double dist_to( const Vec2& other ) const {
                return std :: sqrt( dist_sq_to( other ) );
            }

        public:
            Vec2 respect_to( const Vec2& other ) const {
                return { x - other.x, y - other.y };
            }

            Vec2 operator () ( const Vec2& other ) const {
                return this -> respect_to( other );
            }

        public:
            Vec2& polar( double angel, double dist ) {
                Rad :: push( angel );

                x += std :: cos( angel ) * dist;
                y += std :: sin( angel ) * dist;

                return *this;
            }

            Vec2 polared( double angel, double dist ) const {
                return Vec2( *this ).polar( angel, dist );
            }


            Vec2& approach( const Vec2 other, double dist ) {
                return polar( other( *this ).angel(), dist );
            }
            
            Vec2 approached( const Vec2 other, double dist ) const {
                return Vec2{ *this }.approach( other, dist );
            }


            Vec2& spin( double theta ) {
                Rad :: push( theta );

                double nx = x * std :: cos( theta ) - y * std :: sin( theta );
                y = x * std :: sin( theta ) + y * std :: cos( theta );
                x = nx;

                return *this;
            }

            Vec2& spin( double theta, const Vec2& other ) {
                *this = this -> respect_to( other ).spin( theta ) + other;

                return *this;
            }

            Vec2 spinned( double theta ) const {
                return Vec2{ *this }.spin( theta );
            }

        public:
            bool is_further_than( const Vec2& other, Heading heading ) const {
                switch( heading ) {
                    case Heading :: NORTH: return y > other.y;
                    case Heading :: EAST:  return x > other.x;
                    case Heading :: SOUTH: return y < other.y;
                    case Heading :: WEST:  return x < other.x;
                }

                return false;
            }

        public:
            template< typename T >
            auto X( const Ray2& ray ) const;

            template< typename T >
            auto X( const Clust2& clust ) const;

        public:
            bool operator == ( const Vec2& other ) const {
                return x == other.x && y == other.y;
            }

            Vec2 operator + ( const Vec2& other ) const {
                return { x + other.x, y + other.y };
            }

            Vec2 operator - ( const Vec2& other ) const {
                return { x - other.x, y - other.y };
            }

            Vec2 operator * ( const Vec2& other ) const {
                return { x * other.x, y * other.y };
            }

            Vec2 operator / ( const Vec2& other ) const {
                return { x / other.x, y / other.y };
            }

            Vec2 operator + ( double delta ) const {
                return { x + delta, y + delta };
            }

            Vec2 operator - ( double delta ) const {
                return { x - delta, y - delta };
            }

            Vec2 operator * ( double delta ) const {
                return { x * delta, y * delta };
            }

            Vec2 operator / ( double delta ) const {
                return { x / delta, y / delta };
            }

            Vec2 operator >> ( double delta ) const {
                return { x + delta, y };
            }

            Vec2 operator ^ ( double delta ) const {
                return { x, y + delta };
            }

            Vec2& operator += ( const Vec2& other ) {
                x += other.x;
                y += other.y;

                return *this;
            }

            Vec2& operator -= ( const Vec2& other ) {
                x -= other.x;
                y -= other.y;

                return *this;
            }

            Vec2& operator *= ( const Vec2& other ) {
                x *= other.x;
                y *= other.y;

                return *this;
            }

            Vec2& operator /= ( const Vec2& other ) {
                x /= other.x;
                y /= other.y;

                return *this;
            }

            Vec2& operator *= ( double delta ) {
                x *= delta;
                y *= delta;

                return *this;
            }

            Vec2& operator /= ( double delta ) {
                x /= delta;
                y /= delta;

                return *this;
            }

            Vec2& operator >>= ( double delta ) {
                x += delta;

                return *this;
            }

            Vec2& operator ^= ( double delta ) {
                y += delta;

                return *this;
            }

            Vec2 operator - () const {
                return ( *this ) * -1.0;
            }

        public:
            static Vec2 O() {
                return { 0.0, 0.0 };
            }

        public:
            void render( Ref< Renderer > renderer, Ref< Is_brush auto > brush ) const;

        };

    

        class Ray2 {
        public:
            Ray2() = default;

            Ray2( Vec2 org, Vec2 v )
                : origin( org ), vec( v ) 
            {}

        public:
            Vec2   origin   = {};
            Vec2   vec      = {};

        public:
            Vec2 end() const {
                return origin + vec;
            }

        public:
            double slope() const {
                return ( end().y - origin.y ) / ( end().x - origin.x );
            }

            std :: tuple< double, double, double > coeffs() const {
                return { vec.y, -vec.x, vec.y * origin.x - vec.x * origin.y };
            }

        public:
            template< typename T >
            auto X( const Vec2& vec ) const {
                this -> X< T >( Ray2{ Vec2 :: O(), vec } );
            }

            template< typename T >
            auto X( const Ray2& other ) const {
                if constexpr( std :: is_same_v< bool, T > ) 
                    return _intersect_vec( other ).has_value();
                else
                    return _intersect_vec( other );
            }

            bool Xprll( const Ray2& other ) const {
                static auto is_normal = [] ( Vec2 normalized ) -> bool {
                    return ( normalized.x >= 0.0 && normalized.x <= 1.0 )
                           ||
                           ( normalized.y >= 0.0 && normalized.y <= 1.0 );
                };


                if( slope() != other.slope() ) return false; 


                return is_normal( other.origin( origin ) / vec )
                       ||
                       is_normal( other.end()( origin ) / vec )
                       ||
                       is_normal( origin( other.origin ) / other.vec )
                       ||
                       is_normal( end()( other.origin ) / other.vec );
            }

            template< typename T >
            auto X( const Clust2& clust ) const;

        private:
            std :: optional< Vec2 > _intersect_vec( const Ray2& other ) const {
                /* Hai noroc nea' Peter +respect. */

                auto [ alpha, bravo, charlie ] = coeffs();
                auto [ delta, echo, foxtrot ] = other.coeffs();

                double golf = alpha * echo - bravo * delta;

                if( golf == 0.0 ) return {};

                Vec2 int_vec = {
                    ( charlie * echo - bravo * foxtrot ) / golf,
                    ( alpha * foxtrot - delta * charlie ) / golf
                };

                Vec2 normalized = int_vec( origin ) / vec;
                
                if( !( ( normalized.x >= 0.0 && normalized.x <= 1.0 )
                    ||
                    ( normalized.y >= 0.0 && normalized.y <= 1.0 ) )
                ) return {};

                normalized = int_vec( other.origin ) / other.vec;

                if( !( ( normalized.x >= 0.0 && normalized.x <= 1.0 )
                    ||
                    ( normalized.y >= 0.0 && normalized.y <= 1.0 ) )
                ) return {};

                return int_vec;
            }

        public:
            void render( Ref< Renderer > renderer, Ref< Is_brush auto > brush ) const;

        };



        class Clust2 {
        public:
            Clust2() = default;

            template< typename Iterator >
            Clust2( Iterator begin, Iterator end ) {
                _vrtx.reserve( std :: abs( std :: distance( begin, end ) ) );

                for( ; begin != end; ++begin )
                    _vrtx.emplace_back( *begin, *begin );
            }

            template< typename Container >
            Clust2( const Container& container )
                : Clust2( container.begin(), container.end() )
            {}

            template< typename T, typename Iterator >
            requires ( 
                std :: is_same_v< std :: decay_t< T >, Vec2 > 
                || 
                std :: is_same_v< std :: decay_t< T >, Clust2* > 
            )
            Clust2( T org, Iterator begin, Iterator end, Vec2 offs = {} ) 
                : Clust2( begin, end ) 
            {
                if constexpr( std :: is_same_v< T, Vec2 > )
                    _origin = org;
                else
                    _origin = { org, offs };
            }

            template< typename T, typename Container >
            requires ( 
                std :: is_same_v< std :: decay_t< T >, Vec2 > 
                || 
                std :: is_same_v< std :: decay_t< T >, Clust2* > 
            )
            Clust2( T org, const Container& container, Vec2 offs = {} ) 
                : Clust2( org, container.begin(), container.end(), offs )
            {}
        
        public:
            Clust2( const Clust2& other )
                : _origin( other._origin ),
                  _vrtx  ( other._vrtx ),
                  _scaleX( other._scaleX ),
                  _scaleY( other._scaleY ),
                  _angel ( other._angel )
            {}

            Clust2& operator = ( const Clust2& other ) { 
                _origin = other._origin;
                _vrtx   = other._vrtx;
                _scaleX = other._scaleX;
                _scaleY = other._scaleY;
                _angel  = other._angel;

                return *this;
            }

            Clust2( Clust2&& other ) noexcept
                : _origin( other._origin ),
                  _vrtx  ( std :: move( other._vrtx ) ),
                  _scaleX( other._scaleX ),
                  _scaleY( other._scaleY ),
                  _angel ( other._angel )
            {}

            Clust2& operator = ( Clust2&& other ) noexcept { 
                _origin = std :: move( other._origin );
                _vrtx   = std :: move( other._vrtx );
                _scaleX = other._scaleX;
                _scaleY = other._scaleY;
                _angel  = other._angel;

                return *this;
            }

        private:
            typedef   std :: variant< Vec2, std :: pair< Clust2*, Vec2 > >   Origin;
            typedef   std :: pair< Vec2, Vec2 >                              Vrtx;

            enum ORIGIN_VARIANT_ACCESS_INDEX {
                VEC = 0,
                HOOK = 1
            };

        private:
            Origin                  _origin   = Vec2{ 0.0, 0.0 };
            std :: vector< Vrtx >   _vrtx     = {};

            double                  _scaleX   = 1.0;
            double                  _scaleY   = 1.0;
            double                  _angel    = 0.0;

        public:
            Vec2 origin() const {
                return is_hooked() ?
                    const_cast< Clust2* >( this ) -> hook().origin() 
                    + 
                    const_cast< Clust2* >( this ) -> hook_offs()
                    :
                    std :: get< VEC >( _origin );
            }

            Vec2& origin_ref() {
                return is_hooked() ?
                    hook().origin_ref()
                    :
                    std :: get< VEC >( _origin );
            }

            operator Vec2 () const {
                return origin();
            }

            Vec2 operator () () const {
                return this -> operator Vec2 ();
            }

        public:
            bool is_hooked() const {
                return _origin.index() == HOOK;
            }

            bool is_hookable_to( Clust2& other ) {
                static bool ( *propagate )( Clust2&, Clust2* ) = [] ( Clust2& clust, Clust2* invoker ) -> bool {
                    if( !clust.is_hooked() ) return true;

                    if( &clust.hook() == invoker ) return false;

                    return propagate( clust.hook(), invoker );
                };

                return propagate( other, this );
            }

            Vec2& hook_offs() {
                return std :: get< HOOK >( _origin ).second;
            }

            Clust2& hook_offs_to( const Vec2& vec ) {
                hook_offs() = vec;

                return *this;
            }

            Clust2& hook() {
                return *std :: get< HOOK >( _origin ).first;
            }

            Clust2& hook_to( Clust2& other, std :: optional< Vec2 > offs = {} ) {
                _origin = std :: make_pair(
                    &other,
                    offs.value_or( origin()( other.origin() ) )
                );

                return *this;
            }

            Clust2& dehook() {
                _origin = origin();

                return *this;
            }

        public:
            Vec2& b_vrtx( size_t idx ) {
                return _vrtx[ idx ].second;
            }

            Vec2& operator [] ( size_t idx ) {
                return _vrtx[ idx ].first;
            }

            Vec2 operator() ( size_t idx ) const {
                return _vrtx[ idx ].first + origin();
            }

            size_t vrtx_count() const {
                return _vrtx.size();
            }

        public:
            double angel() const {
                return _angel;
            }

            double scaleX() const {
                return _scaleX;
            }

            double scaleY() const {
                return _scaleY;
            }

            double scale() const {
                return scaleX();
            }

        public:
            Clust2& operator = ( const Vec2& vec ) {
                return relocate( vec );
            }

            Clust2& relocate( const Vec2& vec ) {
                origin_ref() = vec;

                return *this;
            }

            Clust2& relocate_by( size_t idx, const Vec2& vec ) {
                origin_ref() += vec.respect_to( this -> operator()( idx ) );

                return *this;
            }

        public:
            Clust2& spin_with( double theta ) {
                _angel += theta;

                _refresh();

                return *this;
            }

            Clust2& spin_at( double theta ) {
                _angel = theta;

                _refresh();

                return *this;
            }

            Clust2& scaleX_with( double delta ) {
                _scaleX *= delta;

                _refresh();

                return *this;
            }

            Clust2& scaleY_with( double delta ) {
                _scaleY *= delta;

                _refresh();

                return *this;
            }

            Clust2& scale_with( double delta ) {
                _scaleX *= delta;
                _scaleY *= delta;

                _refresh();

                return *this;
            }

            Clust2& scaleX_at( double delta ) {
                _scaleX = delta;

                _refresh();

                return *this;
            }

            Clust2& scaleY_at( double delta ) {
                _scaleY = delta;

                _refresh();

                return *this;
            }

            Clust2& scale_at( double delta ) {
                _scaleX = _scaleY = delta;

                _refresh();

                return *this;
            }

        public:
            static Clust2 triangle( double edge_length ) {
                Vec2 vrtx = { 0.0, edge_length * std :: sqrt( 3.0 ) / 3.0 };

                return std :: vector< Vec2 >( {
                    vrtx,
                    vrtx.spinned( 120.0 ),
                    vrtx.spinned( -120.0 )
                } );
            }

            static Clust2 square( double edge_length ) {
                edge_length /= 2.0;

                return std :: vector< Vec2 >( {
                    { edge_length, edge_length },
                    { edge_length, -edge_length },
                    { -edge_length, -edge_length },
                    { -edge_length, edge_length }
                } );
            }

            static Clust2 circle( double radius, size_t precision ) {
                std :: vector< Vec2 > vrtx;
                vrtx.reserve( precision );

                vrtx.emplace_back( 0.0, radius );

                for( size_t n = 1; n < precision; ++n )
                    vrtx.push_back( vrtx.front().spinned( 360.0 / precision * n ) );

                return vrtx;
            }

            static Clust2 rect( Vec2 tl, Vec2 br ) {
                return std :: vector< Vec2 >( {
                    tl, Vec2( br.x, tl.y ), br, Vec2( tl.x, br.y )
                } );
            }

            template< typename T >
            requires std :: is_invocable_v< T >
            static Clust2 random( 
                double min_dist, double max_dist, 
                size_t min_ec, size_t max_ec, 
                const T& generator 
            ) {
                static auto scalar = [] ( const auto& generator, double min ) -> double {
                    return ( static_cast< double >( std :: invoke( generator ) % 10001 ) / 10000 )
                           * ( 1.0 - min ) + min;
                };

                size_t edge_count = std :: invoke( generator ) % ( max_ec - min_ec + 1 ) + min_ec;

                Vec2 vrtx[ edge_count ];

                vrtx[ 0 ] = { 0.0, max_dist };

                double diff = 360.0 / edge_count;


                for( size_t n = 1; n < edge_count; ++n )
                    vrtx[ n ] = vrtx[ 0 ].spinned( diff * n + ( scalar( generator, min_dist / max_dist ) - 0.5 ) * diff )
                                *
                                scalar( generator, min_dist / max_dist );


                return { vrtx, vrtx + edge_count };
            }

            //static Clust2 sprite( View< Sprite > sprite );

        public:
            template< Word W >
            requires ( W == REF || W == IDX )
            typename std :: conditional_t< W == REF, Vec2&, size_t > extreme( Heading heading ) {
                size_t ex_idx = 0;

                for( size_t idx = 0; idx < vrtx_count(); ++idx )
                    if( _vrtx[ idx ].first.is_further_than( _vrtx[ ex_idx ].first, heading ) )
                        ex_idx = idx;

                if constexpr( W == REF )
                    return _vrtx[ ex_idx ].first;
                else
                    return ex_idx;
            }

            Vec2 extreme( Heading heading, System system = GLOBAL ) const {
                return const_cast< Clust2* >( this ) -> extreme< REF >( heading ) 
                       + 
                       ( system == GLOBAL ? origin() : Vec2 :: O() );
            }

        public:
            template< typename T >
            auto X( const Vec2& vec ) const {
                return this -> X< T >( Ray2{ Vec2 :: O(), vec } );
            }

            template< typename T >
            auto X( const Ray2& ray ) const {
                if constexpr( std :: is_same_v< bool, T > )
                    return _intersect_ray_bool( ray );
                else
                    return _intersect_ray_vec( ray );
            }

            template< typename T >
            auto X( const Clust2& other ) const { 
                if constexpr( std :: is_same_v< bool, T > )
                    return _intersect_bool( other ); 
                else
                    return _intersect_vec( other );
            }

        private:
            bool _intersect_ray_bool( const Ray2& ray ) const {
                for( size_t idx = 0; idx < vrtx_count(); ++idx )
                    if( _mkray( idx ).X< bool >( ray ) )
                        return true;

                return false;
            }

            std :: vector< Vec2 > _intersect_ray_vec( const Ray2& ray ) const {
                std :: vector< Vec2 > Xs{};

                for( size_t idx = 0; idx < vrtx_count(); ++idx ) {
                    auto vec = _mkray( idx ).X< Vec2 >( ray );

                    if( vec.has_value() )
                        Xs.push_back( vec.value() );
                }

                return Xs;
            }

            bool _intersect_bool( const Clust2& other ) const {
                for( size_t idx = 0; idx < other.vrtx_count(); ++idx )
                    if( this -> X< bool >( other._mkray( idx ) ) )
                        return true;

                return false;
            }

            std :: vector< Vec2 > _intersect_vec( const Clust2& other ) const {
                std :: vector< Vec2 > Xs{};

                for( size_t idx = 0; idx < other.vrtx_count(); ++idx ) {
                    auto vecs = this -> X< Vec2 >( other._mkray( idx ) );

                    Xs.insert( Xs.end(), vecs.begin(), vecs.end() );
                }

                return Xs;
            }
        
        public:
            bool contains( const Vec2& vec ) const {
                Ray2 strike = { 
                    vec, 
                    ( vec >> ( vec.dist_sq_to( extreme( EAST, GLOBAL ) ) + 10.0 ) )( vec )
                };

                size_t intersections = 0;
                
                std :: pair< bool, bool > skip = { false, false };

                
                for( size_t idx = 0; idx < vrtx_count(); ++idx ) {
                    Ray2 edge = _mkray( idx, ( idx + 1 ) % vrtx_count() );
                    
                    if( skip.first ) {
                        skip.first = false;

                        if( skip.second ? edge.end().y <= strike.origin.y : edge.end().y >= strike.origin.y )
                            continue;
                    }  

                    if( strike.X< bool >( edge ) ) {
                        ++intersections;

                        if( skip.first = ( edge.end().y == strike.origin.y ) )
                            skip.second = edge.origin.y > strike.origin.y; 
                    }
                }

                if( skip.first && !strike.Xprll( _mkray( 0, 1 ) ) )
                    --intersections;
                
                return intersections % 2;
            }

        private:
            void _refresh() {
                for( Vrtx& v : _vrtx ) 
                    v.first = v.second.spinned( _angel ) * Vec2( _scaleX, _scaleY );
            }

            Ray2 _mkray( size_t idx ) const {
                return { ( *this )( idx ), ( *this )( ( idx + 1 ) % vrtx_count() )( ( *this )( idx ) ) };
            }

            Ray2 _mkray( size_t idx1, size_t idx2 ) const {
                return { ( *this )( idx1 ), ( *this )( idx2 )( ( *this )( idx1 ) ) };
            }
        
        public:
            static Clust2 from_file( std :: string_view path ) { 
                std :: ifstream file( path.data() );

                if( !file ) return {};

                Vec2 org = {};

                std :: vector< Vec2 > vrtx = {};

                {
                    bool has_origin = false;

                    file >> has_origin;

                    if( has_origin )
                        file >> org.x >> org.y;
                }

                for( Vec2 vec; file >> vec.x >> vec.y; )
                    vrtx.push_back( vec );

                return { org, vrtx };
            }
    
        public:
            void render( Ref< Renderer > renderer, Ref< Is_brush auto > brush ) const;

        };



        template< typename T >
        auto Vec2 :: X( const Ray2& ray ) const {
            return ray.X< T >( *this );
        }

        template< typename T >
        auto Vec2 :: X( const Clust2& clust ) const {
            return clust.X< T >( *this );
        }


        template< typename T >
        auto Ray2 :: X( const Clust2& clust ) const {
            return clust.X< T >( *this );
        }

        

    #pragma endregion D2

#pragma endregion Space



#pragma region Graphics

    #pragma region Surface

        class Key {
        public:
            enum Value {
                NONE = 0,

                LMB = 259, RMB, MMB,

                _0 = 48, _1, _2, _3, _4, _5, _6, _7, _8, _9,

                A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

                F1 = 112, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

                CTRL = 17, SHIFT = 16, ALT = 18, TAB = 9, CAPS = 20, ESC = 27, BACKSPACE = 8, ENTER = 13,

                SPACE = 32, DOT = 190, COMMA = 188, COLON = 186, APOSTH = 222, DASH = 189, EQUAL = 187, UNDER_ESC = 192,

                OPEN_BRACKET = 219, CLOSED_BRACKET = 221, BACKSLASH = 220, SLASH = 191,

                ALEFT = 37, AUP, ARIGHT, ADOWN
            };

            enum State {
                UP, DOWN
            };

            static constexpr size_t   COUNT   = 262;

        public:
            Key() = default;

            Key( short key ) 
                : value( key ) 
            {}

        public:
            short   value   = NONE;

        public:
            operator short () const {
                return value;
            }

        public:
            bool operator == ( const Key& other ) const {
                return value == other.value;
            }

            bool operator == ( Value val ) const {
                return value == val;
            }

            std :: strong_ordering operator <=> ( const Key& other ) const {
                return value <=> other.value;
            }

            std :: strong_ordering operator <=> ( Value val ) const {
                return value <=> val;
            }

        public:
        #if defined( _ENGINE_ONE_SURFACE )
            template< typename ...Keys > static size_t any_down( Keys... keys );

            template< typename ...Keys > static size_t smr_down( Keys... keys );

            template< typename ...Keys > static bool all_down( Keys... keys );
            
            static bool down( Key key );
        #endif

        };


        
        class Scroll {
        public:
            enum Dir {
                UP, DOWN, LEFT, RIGHT
            };

        };



        class Surface : public Has_op_ptr< Surface > {
        public:
            inline static const char*   name   = _ENGINE_CLASS_NAME( "Surface" );

        private:
            friend class Key;
            friend class Mouse;
            friend class Renderer;

        public:
            enum Event {
                KEY, MOUSE, SCROLL, FILEDROP, MOVE, RESIZE,

                _DESTROY = 69100,
                _CURSOR_HIDE, _CURSOR_SHOW,
                _FORCE
            };

            enum Plug {
                BEFORE = 0, AFTER
            };

        public:
            Surface() = default;

            Surface( 
                std :: string_view title, 
                Coord< int >       coord = { 0, 0 }, 
                Size< int >        size  = { 512, 512 },
                Echo               echo  = {} 
            )  
                : _coord( coord ), _size( size )
            {
                #if defined( _ENGINE_ONE_SURFACE )
                    _ptr = this;
                #endif

                _wnd_class.cbSize        = sizeof( WNDCLASSEX );
                _wnd_class.hInstance     = GetModuleHandle( NULL );
                _wnd_class.lpfnWndProc   = event_proc_router;
                _wnd_class.lpszClassName = title.data();
                _wnd_class.hbrBackground = HBRUSH( COLOR_INACTIVECAPTIONTEXT );
                _wnd_class.hCursor       = LoadCursor( NULL, IDC_ARROW );


                std :: binary_semaphore sync{ 0 };

                _thread = std :: thread( _main, this, &sync, echo );

                if( _thread.joinable() ) {
                    echo( this, Echo :: PENDING, "Awaiting window creation..." );

                    sync.acquire();
                } else
                    echo( this, Echo :: FAULT, "Window thread launch failed." );
            }

            /* _Surface... Heh */
            ~Surface() {
                SendMessage( _hwnd, Event :: _DESTROY, NULL, NULL );

                if( _thread.joinable() )
                    _thread.join();

                #if defined( _ENGINE_ONE_SURFACE )
                    _ptr = nullptr;
                #endif
            }

        public:
            struct Trace {
                Trace() = default;

                struct Result {
                    UID                   uid      = NULL;
                    std :: bitset< 16 >   result   = 0;
                };
                
                std :: bitset< 64 >       master   = 0;
                std :: vector< Result >   plugs    = {};

                void clear() {
                    master.reset();
                    plugs.clear();
                }

                std :: bitset< 64 > :: reference operator [] ( size_t idx ) {
                    return master[ idx ];
                }
            };

        public:
            typedef   std :: function< void( Vec2, Vec2, Ref< Trace > ) >                       On_mouse;
            typedef   std :: function< void( Key, Key :: State, Ref< Trace > ) >                On_key;
            typedef   std :: function< void( Scroll :: Dir, Ref< Trace > ) >                    On_scroll;
            typedef   std :: function< void( std :: vector< std :: string >, Ref< Trace > ) >   On_filedrop;
            typedef   std :: function< void( Coord< int >, Coord< int >, Ref< Trace > ) >       On_move;
            typedef   std :: function< void( Size< int >, Size< int >, Ref< Trace > ) >         On_resize;

            typedef   std :: array< Key :: State, Key :: COUNT >                                Keys;

        private:
            static constexpr int   LIQUID_STYLE   = WS_OVERLAPPED | WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
            static constexpr int   SOLID_STYLE    = WS_POPUP | WS_VISIBLE;

        private:
        #if defined( _ENGINE_ONE_SURFACE )
            inline static Ptr< Surface >         _ptr                  = nullptr;
        #endif
            HWND                                 _hwnd                 = nullptr;
            WNDCLASSEX                           _wnd_class            = {};
            std :: thread                        _thread               = {};
            Coord< int >                         _coord                = {};
            Size< int >                          _size                 = {};

            Trace                                _trace                = {};

            On_mouse                             _on_mouse             = {};
            On_key                               _on_key               = {};
            On_scroll                            _on_scroll            = {};
            On_filedrop                          _on_filedrop          = {};
            On_move                              _on_move              = {};
            On_resize                            _on_resize            = {};

            std :: map< UID, On_mouse >          _plug_mouse[ 2 ]      = {};
            std :: map< UID, On_key >            _plug_key[ 2 ]        = {};
            std :: map< UID, On_scroll >         _plug_scroll[ 2 ]     = {};
            std :: map< UID, On_filedrop >       _plug_filedrop[ 2 ]   = {};
            std :: map< UID, On_move >           _plug_move[ 2 ]       = {};
            std :: map< UID, On_resize >         _plug_resize[ 2 ]     = {};
            
     
            Vec2                                 _mouse                = {};
            Vec2                                 _mouse_l              = {};
            Keys                                 _keys                 = {};

        private:
            void _main( std :: binary_semaphore* sync, Echo echo = {} ) {
                if( !RegisterClassEx( &_wnd_class ) ) { 
                    echo( this, Echo :: FAULT, "Window class registration failed." );

                    sync -> release(); return;
                }


                _hwnd = CreateWindowEx(
                    WS_EX_ACCEPTFILES,

                    _wnd_class.lpszClassName, _wnd_class.lpszClassName,

                    SOLID_STYLE,

                    _coord.x, _coord.y, _size.width, _size.height,

                    NULL, NULL,

                    GetModuleHandle( NULL ),

                    this
                );

                if( !_hwnd ) {
                    echo( this, Echo :: FAULT, "Window creation failed." );

                    sync -> release(); return;
                }

                SetWindowText( _hwnd, _wnd_class.lpszClassName );


                echo( this, Echo :: OK, "Created." );

                sync -> release();


                MSG event;

                while( GetMessage( &event, NULL, 0, 0 ) > 0 ) {
                    TranslateMessage( &event );
                    DispatchMessage( &event );
                }

            }

            static LRESULT CALLBACK event_proc_router( HWND hwnd, UINT event, WPARAM w_param, LPARAM l_param ) {
                #if defined( _ENGINE_ONE_SURFACE )
                    if( event == WM_NCCREATE )
                        static_cast< Ptr< Surface > >( 
                            reinterpret_cast< LPCREATESTRUCT >( l_param ) -> lpCreateParams 
                        ) -> _hwnd = hwnd;
                    else
                        if( _ptr )
                            return _ptr -> event_proc( hwnd, event, w_param, l_param );
                #else
                    Ptr< Surface > ptr = nullptr;

                    if( event == WM_NCCREATE ){
                        ptr = static_cast< Ptr< Surface > >( reinterpret_cast< LPCREATESTRUCT >( l_param ) -> lpCreateParams );
                        ptr -> _hwnd = hwnd;
                        SetWindowLongPtr( hwnd, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( ptr ) );
                    }
                    else
                        ptr = reinterpret_cast< Ptr< Surface > >( GetWindowLongPtr( hwnd, GWLP_USERDATA ) );

                    if( ptr ) 
                        return ptr -> event_proc( hwnd, event, w_param, l_param );
                #endif

                return DefWindowProc( hwnd, event, w_param, l_param );
            }

            LRESULT CALLBACK event_proc( HWND hwnd, UINT event, WPARAM w_param, LPARAM l_param ) {
                static auto key_proc = [ this ] ( Key :: State state, WPARAM w_param ) -> void {
                    Key key = static_cast< Key >( w_param );

                    _keys[ key ] = state;
                    
                    invoke_ons< On_key >( key, state );
                };

                switch( event ) {
                    case WM_CREATE: {

                    break; }


                    case Event :: _DESTROY: {
                        DestroyWindow( hwnd );
                        PostQuitMessage( NULL );
                    break; }

                    case Event :: _CURSOR_HIDE: {
                        while( ShowCursor( false ) >= 0 );
                    break; }

                    case Event :: _CURSOR_SHOW: {
                        while( ShowCursor( true ) >= 0 );
                    break; }

                    case Event :: _FORCE :{
                        invoke_ons< On_mouse >( _mouse, _mouse );
                    break; }


                    case WM_MOUSEMOVE: {
                        Vec2 new_mouse = pull_vec( { LOWORD( l_param ), HIWORD( l_param ) } );

                        invoke_ons< On_mouse >( new_mouse, _mouse_l = std :: exchange( _mouse, new_mouse ) );

                    break; }

                    case WM_MOUSEWHEEL: {
                        invoke_ons< On_scroll >( 
                            GET_WHEEL_DELTA_WPARAM( w_param ) < 0 
                            ? 
                            Scroll :: Dir :: DOWN : Scroll :: Dir :: UP 
                        );

                        break;
                    }


                    case WM_LBUTTONDOWN: {
                        key_proc( Key :: State :: DOWN, Key :: LMB );

                        break;
                    }

                    case WM_LBUTTONUP: {
                        key_proc( Key :: State :: UP, Key :: LMB );

                        break;
                    }

                    case WM_RBUTTONDOWN: {
                        key_proc( Key :: State :: DOWN, Key :: RMB );

                        break;
                    }

                    case WM_RBUTTONUP: {
                        key_proc( Key :: State :: UP, Key :: RMB );

                        break;
                    }

                    case WM_MBUTTONDOWN: {
                        key_proc( Key :: State :: DOWN, Key :: MMB );

                        break;
                    }

                    case WM_MBUTTONUP: {
                        key_proc( Key :: State :: UP, Key :: MMB );

                        break;
                    }

                    case WM_KEYDOWN: {
                        if( l_param & ( 1 << 30 ) ) break;

                        key_proc( Key :: State :: DOWN, w_param );

                        break;
                    }

                    case WM_KEYUP: {
                        key_proc( Key :: State :: UP, w_param );

                        break;
                    }


                    case WM_DROPFILES: {
                        size_t file_count = DragQueryFile( reinterpret_cast< HDROP >( w_param ), -1, NULL, NULL );

                        std :: vector< std :: string > files;

                        for( size_t n = 0; n < file_count; ++ n ) {
                            TCHAR file[ MAX_PATH ];

                            DragQueryFile( reinterpret_cast< HDROP >( w_param ), n, file, MAX_PATH );

                            files.emplace_back( file );
                        }

                        invoke_ons< On_filedrop >( std :: move( files ) );

                    break; }


                    case WM_MOVE: {
                        Coord< int > new_coord = { LOWORD( l_param ), HIWORD( l_param ) };

                        invoke_ons< On_move >( new_coord, std :: exchange( _coord, new_coord ) );                    

                    break; }

                    case WM_SIZE: {
                        Size< int > new_size = { LOWORD( l_param ), HIWORD( l_param ) };

                        invoke_ons< On_resize >( new_size, std :: exchange( _size, new_size ) );

                        break; }

                }

                return DefWindowProc( hwnd, event, w_param, l_param );
            }

            template< typename On, typename ...Args >
            void invoke_ons( Args&&... args ) {
                if constexpr( std :: is_same_v< On, On_mouse > )
                    invoke_ons( _on_mouse, _plug_mouse, std :: forward< Args >( args )... );

                else if constexpr( std :: is_same_v< On, On_key > )
                    invoke_ons( _on_key, _plug_key, std :: forward< Args >( args )... );

                else if constexpr( std :: is_same_v< On, On_scroll > )
                    invoke_ons( _on_scroll, _plug_scroll, std :: forward< Args >( args )... );

                else if constexpr( std :: is_same_v< On, On_filedrop > )
                    invoke_ons( _on_filedrop, _plug_filedrop, std :: forward< Args >( args )... );

                else if constexpr( std :: is_same_v< On, On_move > )
                    invoke_ons( _on_move, _plug_move, std :: forward< Args >( args )... );

                else if constexpr( std :: is_same_v< On, On_resize > )
                    invoke_ons( _on_resize, _plug_resize, std :: forward< Args >( args )... );
                            
            }

            template< typename M, typename P, typename ...Args >
            void invoke_ons( M& master, P& plugs, Args&&... args ) {
                _trace.clear();

                for( auto& pair : plugs[ 0 ] )
                    std :: invoke( pair.second, std :: forward< Args >( args )..., _trace );

                if( master ) 
                    std :: invoke( master, std :: forward< Args >( args )..., _trace );

                for( auto& pair : plugs[ 1 ] )
                    std :: invoke( pair.second, std :: forward< Args >( args )..., _trace );
            }

        public:
            Vec2 pull_vec( const Coord< float >& coord ) const {
                return { coord.x - _size.width / 2.0, _size.height / 2.0 - coord.y };
            }

            Coord< float > pull_coord( const Vec2& vec ) const {
                return { vec.x + _size.width / 2.0, _size.height / 2.0 - vec.y };
            }

            void push_vec( Coord< float >& coord ) const {
                coord.x -= _size.width / 2.0;
                coord.y = _size.height / 2.0 - coord.y;
            }

            void push_coord( Vec2& vec ) const {
                vec.x += _size.width / 2.0; 
                vec.y = _size.height / 2.0 - vec.y;
            }

        public:
            Coord< int > pos() {
                RECT rect = {};

                GetWindowRect( _hwnd, &rect );
            
                return { rect.left, rect.top };
            }

            int x() {
                return pos().x;
            }

            int y() {
                return pos().y;
            }

            Size< int > size() {
                RECT rect = {};

                GetWindowRect( _hwnd, &rect );
            
                return { rect.right - rect.left, rect.bottom - rect.top };
            }

            int width() {
                return size().width;
            }

            int height() {
                return size().height;
            }

        public:
            Surface& solidify() {
                SetWindowLongPtr( _hwnd, GWL_STYLE, SOLID_STYLE );

                return *this;
            }

            Surface& liquify() {
                SetWindowLongPtr( _hwnd, GWL_STYLE, LIQUID_STYLE );

                return *this;
            }

            Surface& move_to( Coord< int > coord ) {
                SetWindowPos(
                    _hwnd,
                    NULL,
                    _coord.x = coord.x, _coord.y = coord.y,
                    NULL, NULL,
                    SWP_NOSIZE
                );

                return *this;
            }

            Surface& size_to( Size< int > size ) {
                SetWindowPos(
                    _hwnd,
                    NULL,
                    NULL, NULL,
                    _size.width = size.width, _size.height = size.height,
                    SWP_NOMOVE
                );

                return *this;
            }

        public:
            Surface& hide_cursor() {
                SendMessage( _hwnd, Event :: _CURSOR_HIDE, NULL, NULL );

                return *this;
            }

            Surface& show_cursor() {
                SendMessage( _hwnd, Event :: _CURSOR_SHOW, NULL, NULL );

                return *this;
            }

        public:
            Vec2 vec() {
                return _mouse;
            }

            Vec2 l_vec() {
                return _mouse_l;
            }

            Coord< int > coord() {
                return pull_coord( vec() );
            }

            Coord< int > l_coord() {
                return pull_coord( l_vec() );
            }

            template< typename ...Keys >
            size_t any_down( Keys... keys ) {
                size_t count = 0;

                ( ( count += ( _keys[ keys ] == Key :: State :: DOWN ) ), ... );

                return count;
            }

            template< typename ...Keys >
            size_t smr_down( Keys... keys ) {
                size_t sum = 0;
                size_t at = 1;

                ( ( sum += 
                    std :: exchange( at, at * 2 )
                    *
                    ( _keys[ keys ] == Key :: State :: DOWN ) 
                ), ... );

                return sum;
            }

            template< typename ...Keys >
            bool all_down( Keys... keys ) { 
                return any_down( keys... ) == sizeof...( Keys );
            }

            bool down( Key key ) {
                return _keys[ key ] == Key :: State :: DOWN;
            }

        public:
            HWND hwnd() {
                return _hwnd;
            }

        public:
            Surface& force() {
                SendMessage( _hwnd, Event :: _FORCE, NULL, NULL );

                return *this;
            }

        public:
            template< Event event, typename T >
            Surface& on( T function ) {
                if      constexpr( event == Event :: MOUSE )    _on_mouse    = function;
                else if constexpr( event == Event :: KEY )      _on_key      = function;
                else if constexpr( event == Event :: SCROLL )   _on_scroll   = function;
                else if constexpr( event == Event :: FILEDROP ) _on_filedrop = function;
                else if constexpr( event == Event :: MOVE )     _on_move     = function;
                else if constexpr( event == Event :: RESIZE )   _on_resize   = function;

                return *this;
            }

            template< Event event, typename T >
            Surface& plug( const UID& uid, Plug priority, T function ) {
                if      constexpr( event == Event :: MOUSE )    _plug_mouse   [ priority ].insert( std :: make_pair( uid, function )  );
                else if constexpr( event == Event :: KEY )      _plug_key     [ priority ].insert( std :: make_pair( uid, function )  );
                else if constexpr( event == Event :: SCROLL )   _plug_scroll  [ priority ].insert( std :: make_pair( uid, function )  );
                else if constexpr( event == Event :: FILEDROP ) _plug_filedrop[ priority ].insert( std :: make_pair( uid, function )  );
                else if constexpr( event == Event :: MOVE )     _plug_move    [ priority ].insert( std :: make_pair( uid, function )  );
                else if constexpr( event == Event :: RESIZE )   _plug_resize  [ priority ].insert( std :: make_pair( uid, function )  );
        
                return *this;
            }   

            Surface& unplug( const UID& uid, std :: optional< Plug > priority = {} ) {
                _unplug( uid, priority, _plug_mouse );
                _unplug( uid, priority, _plug_key );
                _unplug( uid, priority, _plug_scroll );
                _unplug( uid, priority, _plug_filedrop );
                _unplug( uid, priority, _plug_move );
                _unplug( uid, priority, _plug_resize );

                return *this;
            }

            template< Event event >
            Surface& unplug( const UID& uid, std :: optional< Plug > priority = {} ) {
                if      constexpr( event == MOUSE )    _unplug( uid, priority, _plug_mouse );
                else if constexpr( event == KEY )      _unplug( uid, priority, _plug_key );
                else if constexpr( event == SCROLL )   _unplug( uid, priority, _plug_scroll );
                else if constexpr( event == FILEDROP ) _unplug( uid, priority, _plug_filedrop );
                else if constexpr( event == MOVE )     _unplug( uid, priority, _plug_move );
                else if constexpr( event == RESIZE )   _unplug( uid, priority, _plug_resize );

                return *this;
            }

        private:
            template< typename T >
            void _unplug( const UID& uid, std :: optional< Plug > priority, T& plug ) {
                if( priority.has_value() )
                    plug[ priority.value() ].erase( uid );
                else {
                    plug[ Plug :: BEFORE ].erase( uid );
                    plug[ Plug :: AFTER ] .erase( uid );
                }
            }

        public:
        #if defined( _ENGINE_ONE_SURFACE )
            static Ptr< Surface > get() {
                return _ptr;
            }
        #endif

        };



        class Mouse {
        public:
        #if defined( _ENGINE_ONE_SURFACE )
            static Vec2 vec() {
                return Surface :: get() -> _mouse;
            }

            static Coord< int > coord() {
                return Surface :: get() -> pull_coord( vec() );
            }
        #endif

        public:
            static Vec2 g_vec() {
                auto [ x, y ] = g_coord();

                return { x - Env :: H_W(), Env :: H_H() - y };
            }

            static Coord< int > g_coord() {
                POINT p;
                GetCursorPos( &p );

                return { p.x, p.y };
            }

        };



        #if defined( _ENGINE_ONE_SURFACE )
            template< typename ...Keys >
            size_t Key :: any_down( Keys... keys ) {
                return Surface :: get() -> any_down( keys... );
            }

            template< typename ...Keys >
            size_t Key :: smr_down( Keys... keys ) {
                return Surface :: get() -> smr_down( keys... );
            }

            template< typename ...Keys >
            bool Key :: all_down( Keys... keys ) { 
                return Surface :: get() -> all_down( keys... );
            }

            bool Key :: down( Key key ) {
                return Surface :: get() -> down( key );
            }
        #endif

    #pragma endregion Surface



    #pragma region Renderer

        class Renderer : public Has_op_ptr< Renderer > {
        public:
            inline static const char*   name   = _ENGINE_CLASS_NAME( "Renderer" );

        private:
            friend class Renderer;

        private:
            friend class Vec2;
            friend class Ray2;
            friend class Clust2;

            friend class Solid_brush;
            friend class Linear_brush;
            friend class Radial_brush;
            friend class Sprite;
            

        public:
            Renderer() = default;

            Renderer( Ptr< Surface > surface, Echo echo = {} ) 
                : _surface{ std :: move( surface ) } 
            {
                HRESULT result;

                result = CoInitialize( nullptr );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Co init failed." ); return;
                }


                result = CoCreateInstance(
                    CLSID_WICImagingFactory,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IWICImagingFactory,
                    ( LPVOID* ) &_wic_factory
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Co instance failed." ); return;
                }


                result = D2D1CreateFactory(
                    D2D1_FACTORY_TYPE_MULTI_THREADED,
                    &_factory
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Factory creation failed." ); return;
                }


                result = _factory -> CreateHwndRenderTarget(
                    D2D1 :: RenderTargetProperties(),
                    D2D1 :: HwndRenderTargetProperties(
                        _surface -> _hwnd,
                        D2D1 :: SizeU( _surface -> width(), _surface -> height() ),
                        D2D1_PRESENT_OPTIONS_IMMEDIATELY
                    ),
                    &_target
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Render target creation failed." ); return;
                }


                push();
                push();


                echo( this, Echo :: OK, "Created." );
            }

        public:
            ~Renderer() { 
                if( _factory ) 
                    _factory -> Release();

                if( _wic_factory ) 
                    _wic_factory -> Release();
                
                if( _target ) 
                    _target -> Release(); 
            }

        public:
            using   Matrix   = D2D1 :: Matrix3x2F;

            struct Parameters {
                float    angel    = 0.0;
                float    scaleX   = 1.0;
                float    scaleY   = 1.0;

                Matrix   mtr      = { Matrix :: Identity() };
            };

        private:
            Ptr< Surface >                 _surface       = {}; 

            Ptr< ID2D1Factory >            _factory       = nullptr;
            Ptr< IWICImagingFactory >      _wic_factory   = nullptr;

            Ptr< ID2D1HwndRenderTarget >   _target        = nullptr;

            std :: vector< Parameters >    _params        = {};

            ///std :: vector< Solid_brush >   _brushes       = {};

        public:
            Renderer& begin() {
                _target -> BeginDraw();

                return *this;
            }

            Renderer& end() {
                _target -> EndDraw();

                return *this;
            }

        public:
            Vec2 pull_vec( const Coord< float >& coord ) const {
                return _surface -> pull_vec( coord );
            }

            Coord< float > pull_coord( const Vec2& vec ) const {
                return _surface -> pull_coord( vec );
            }

            void push_vec( Coord< float >& coord ) const {
                _surface -> push_vec( coord );
            }

            void push_coord( Vec2& vec ) const {
                _surface -> push_coord( vec );
            }


        private:
            Parameters& _now() {
                return _params.back();
            }

            Parameters& _before() {
                return *----_params.end();
            }

        public:
            Renderer& apply_around( Coord< float > crd ) {
                auto& [ angel, scaleX, scaleY, mtr ] = _now();

                mtr = Matrix :: Rotation( 
                          -angel, crd
                      )
                      *
                      Matrix :: Scale( 
                          scaleX, scaleY, crd
                      )
                      *
                      _before().mtr;

                _target -> SetTransform( mtr );

                return *this;
            }

            Renderer& apply_around( Vec2 vec ) {
                return apply_around( pull_coord( vec ) );
            }

        public:
            Renderer& spin_with( float theta ) {
                _now().angel += theta;

                return *this;
            }

            Renderer& spin_at( float theta ) {
                _now().angel = theta;

                return *this;
            }

            Renderer& scaleX_with( float delta ) {
                _now().scaleX *= delta;

                return *this;
            }

            Renderer& scaleX_at( float delta ) {
                _now().scaleX = delta;

                return *this;
            }

            Renderer& scaleY_with( float delta ) {
                _now().scaleY *= delta;

                return *this;
            }

            Renderer& scaleY_at( float delta ) {
                _now().scaleY = delta;

                return *this;
            }

            Renderer& scale_with( float delta ) {
                scaleX_with( delta );
                return scaleY_with( delta );
            }

            Renderer& scale_at( float delta ) {
                scaleX_at( delta );
                return scaleY_at( delta );
            }

        public:
            Renderer& push() {
                _params.emplace_back();

                return *this;
            }

            Renderer& push( const Parameters& params ) {
                _params.emplace_back( params );

                return *this;
            }

            Renderer& pop() {
                _params.pop_back();

                return *this;
            }

        public:
            Renderer& fill( const Chroma& chroma );

        public:
            Renderer& system( Ref< Is_brush auto > brush );

        public:
            Renderer& line(
                Coord< float > c1, Coord< float > c2,
                Ref< Is_brush auto > brush
            );

            Renderer& line(
                Vec2 v1, Vec2 v2,
                Ref< Is_brush auto > brush
            );

        public:
            template< typename T, typename ...Args >
            Renderer& operator () ( const T& thing, Args&&... args ) {
                thing.render( *this, std :: forward< Args >( args )... );

                return *this;
            }

        };

    #pragma endregion Renderer



    #pragma region Brushes

        class Chroma {
        public:
            Chroma() = default;

            Chroma( float r, float g, float b, float a = 1.0 )
                : r( r ), g( g ), b( b ), a( a ) 
            {}

        public:
            float   r   = 0.0;
            float   g   = 0.32;   /* dark verdian for nyjucu aka iupremacy */
            float   b   = 0.23;
            float   a   = 1.0;

        public:
            operator const D2D1_COLOR_F& () const {
                return *reinterpret_cast< const D2D1_COLOR_F* >( this );
            } 

            operator D2D1_COLOR_F& () {
                return *reinterpret_cast< D2D1_COLOR_F* >( this );
            } 


        };



        class Solid_brush {
        public:
            inline static const char*   name   = _ENGINE_CLASS_NAME( "Solid_brush" );

        private:
            friend class Renderer;

        public:
            Solid_brush() = default;

            Solid_brush( 
                Ref< Renderer > renderer, 
                Chroma          chroma   = {}, 
                float           width    = 3.0,
                Echo            echo     = {} 
            )
                : _width( width ) 
            {
                HRESULT result = renderer._target -> CreateSolidColorBrush(
                    chroma,
                    &_brush
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Brush creation failed." ); return;
                }

                echo( this, Echo :: OK, "Created." ); 
            }

        public:
            ~Solid_brush() {
                if( _brush ) _brush -> Release();
            }

        private:
            ID2D1SolidColorBrush*   _brush   = nullptr;

            float                   _width   = 1.0;

        public:
            float width() const {
                return _width;
            }

            Chroma chroma() const {
                auto [ r, g, b, a ] = _brush -> GetColor();
                return { r, g, b, a };
            }

            float r() const {
                return chroma().r;
            }

            float g() const {
                return chroma().g;
            }

            float b() const {
                return chroma().b;
            }

            float opac() const {
                return chroma().a;
            }

        public:
            Solid_brush& width_to( float w ) {
                _width = w;

                return *this;
            }

            Solid_brush& chroma_to( Chroma c ) {
                _brush -> SetColor( c );

                return *this;
            }

            Solid_brush& r_to( float value ) {
                return chroma_to( { value, g(), b() } );
            }

            Solid_brush& g_to( float value ) {
                return chroma_to( { r(), value, b() } );
            }

            Solid_brush& b_to( float value ) {
                return chroma_to( { r(), g(), value } );
            }

            Solid_brush& opac_to( float value ) {
                _brush -> SetOpacity( value );

                return *this;
            }

        };



        class Linear_brush {
        public:
            inline static const char*   name   = _ENGINE_CLASS_NAME( "Linear_brush" );

        private:
            friend class Renderer;

        public:
            Linear_brush() = default;

            template< typename Itr >
            Linear_brush( 
                Ptr< Renderer > renderer, 
                Vec2            tl, 
                Vec2            br,
                Itr             begin, 
                Itr             end,
                float           width    = 3.0,
                Echo            echo     = {} 
            )
                : _renderer( std :: move( renderer ) ), _width( width ) 
            {
                HRESULT result;


                D2D1_GRADIENT_STOP entries[ std :: abs( std :: distance( begin, end ) ) ];


                std :: size_t idx = 0;

                for( Itr itr = begin; itr != end; ++itr, ++idx ) {
                    entries[ idx ].color = itr -> first;
                    entries[ idx ].position = itr -> second;
                };


                result = _renderer -> _target -> CreateGradientStopCollection(
                    entries,
                    idx,
                    D2D1_GAMMA_2_2,
                    D2D1_EXTEND_MODE_CLAMP,
                    &_grads
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Grads creation failed." ); return;
                }


                result = _renderer -> _target -> CreateLinearGradientBrush(
                    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES{
                        _renderer -> _surface -> pull_coord( tl ), 
                        _renderer -> _surface -> pull_coord( br )
                    },
                    D2D1_BRUSH_PROPERTIES {
                        1.0,
                        D2D1 :: Matrix3x2F :: Identity()
                    },
                    _grads,
                    &_brush
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Brush creation failed." ); return;
                }


                echo( this, Echo :: OK, "Created." );
            }

            template< typename Cntr >
            Linear_brush(
                Ptr< Renderer > renderer,
                Vec2            tl, 
                Vec2            br,
                const Cntr&     container,
                float           width     = 3.0
            )
                : Linear_brush( renderer, tl, br, container.begin(), container.end(), width )
            {}

        public:
            ~Linear_brush() {
                if( _brush ) _brush -> Release();

                if( _grads ) _grads -> Release();
            }

        private:
            Ptr< Renderer >                _renderer   = nullptr;

            ID2D1LinearGradientBrush*      _brush      = nullptr;

            ID2D1GradientStopCollection*   _grads      = nullptr;

            float                          _width      = 1.0;

        public:
            float width() const {
                return _width;
            }
            
            Vec2 topleft() const {
                auto [ x, y ] = _brush -> GetStartPoint();
                return _renderer -> _surface -> pull_vec( Coord< float >{ x, y } );
            }

            Vec2 botright() const {
                auto [ x, y ] = _brush -> GetEndPoint();
                return _renderer -> _surface -> pull_vec( Coord< float >{ x, y } );
            }

        public:
            Linear_brush& width_to( float w ) {
                _width = w;

                return *this;
            }

            Linear_brush& topleft_to( Vec2 tl ) {
                _brush -> SetStartPoint( _renderer -> _surface -> pull_coord( tl ) );
                
                return *this;
            }

            Linear_brush& botright_to( Vec2 br ) {
                _brush -> SetEndPoint( _renderer -> _surface -> pull_coord( br ) );
                
                return *this;
            }

        };



        class Radial_brush {
        public:
            inline static const char*   name   = _ENGINE_CLASS_NAME( "Radial_brush" );

        private:
            friend class Renderer;

        public:
            Radial_brush() = default;

            template< typename Itr >
            Radial_brush( 
                Ptr< Renderer > renderer, 
                Vec2            c, 
                Vec2            offs,
                float           rx,
                float           ry,
                Itr             begin, 
                Itr             end,
                float           width    = 3.0,
                Echo            echo     = {} 
            )
                : _renderer{ std :: move( renderer ) }, _width{ width } 
            {
                HRESULT result;


                D2D1_GRADIENT_STOP entries[ std :: abs( std :: distance( begin, end ) ) ];


                std :: size_t idx = 0;

                for( Itr itr = begin; itr != end; ++itr, ++idx ) {
                    entries[ idx ].color = itr -> first;
                    entries[ idx ].position = itr -> second;
                };


                result = _renderer -> _target -> CreateGradientStopCollection(
                    entries,
                    idx,
                    D2D1_GAMMA_2_2,
                    D2D1_EXTEND_MODE_CLAMP,
                    &_grads
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Grads creation failed." ); return;
                }

                result = _renderer -> _target -> CreateRadialGradientBrush(
                    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES{
                        _renderer -> _surface -> pull_coord( c ), 
                        Coord< float >{ offs.x, -offs.y }, 
                        rx, ry
                    },
                    _grads,
                    &_brush
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Brush creation failed." ); return;
                }


                echo( this, Echo :: OK, "Created." );
            }

            template< typename Cntr >
            Radial_brush(
                Ptr< Renderer > renderer,
                Vec2            c, 
                Vec2            offs,
                float           rx,
                float           ry,
                const Cntr&     container,
                float           width     = 3.0
            )
                : Radial_brush{ 
                      renderer, 
                      c, offs, rx, ry,
                      container.begin(), container.end(), 
                      width 
                }
            {}

        public:
            ~Radial_brush() {
                if( _brush ) _brush -> Release();

                if( _grads ) _grads -> Release();
            }

        private:
            Ptr< Renderer >                _renderer   = nullptr;

            ID2D1RadialGradientBrush*      _brush      = nullptr;

            ID2D1GradientStopCollection*   _grads      = nullptr;

            float                          _width      = 1.0;

        public:
            float width() const {
                return _width;
            }
            
            Vec2 center() const {
                auto [ x, y ] = _brush -> GetCenter();
                return _renderer -> _surface -> pull_vec( Coord< float >{ x, y } );
            }

            Vec2 offset() const {
                auto [ x, y ] = _brush -> GetGradientOriginOffset();
                return { x, -y };
            }

            float radX() const {
                return _brush -> GetRadiusX();
            }

            float radY() const {
                return _brush -> GetRadiusY();
            }

            Vec2 rad() const {
                return { radX(), radY() };
            }

        public:
            Radial_brush& width_to( float w ) {
                _width = w;

                return *this;
            }

            Radial_brush& center_to( Vec2 c ) {
                _brush -> SetCenter( _renderer -> _surface -> pull_coord( c ) );
                
                return *this;
            }

            Radial_brush& offset_to( Vec2 offs ) {
                _brush -> SetGradientOriginOffset( Coord< float >{ offs.x, -offs.y } );
                
                return *this;
            }

            Radial_brush& radX_to( float rx ) {
                _brush -> SetRadiusX( rx );

                return *this;            
            }

            Radial_brush& radY_to( float ry ) {
                _brush -> SetRadiusY( ry );

                return *this;            
            }

            Radial_brush& rad_to( Vec2 vec ) {
                radX_to( vec.x );
                radY_to( vec.y );

                return *this;            
            }

        };

    #pragma endregion Brushes


    
    #pragma region Sprite

        class Sprite : public Rendable,
                       public Has_op_ptr< Sprite >
        {
        public:
            constexpr static char*   name   = "Gtl :: Sprite";

        public:
            Sprite() = default;

            Sprite( 
                Ref< Renderer >    renderer, 
                std :: string_view path, 
                Echo               echo     = {} 
            ) 
                : _path( path )
            {
                using namespace std :: string_literals;


                HRESULT result;

                IWICBitmapDecoder*     wic_decoder   = nullptr;
                IWICBitmapFrameDecode* wic_frame     = nullptr;
                IWICFormatConverter*   wic_converter = nullptr;


                auto kill_wics = [ &wic_decoder, &wic_frame, &wic_converter ] () -> void {
                    if( wic_converter ) wic_converter -> Release();
                    if( wic_decoder ) wic_decoder -> Release();
                    if( wic_frame ) wic_frame -> Release();
                };


                result = renderer._wic_factory -> CreateDecoderFromFilename(
                    std :: wstring( path.begin(), path.end() ).c_str(),
                    nullptr,
                    GENERIC_READ,
                    WICDecodeOptions :: WICDecodeMetadataCacheOnLoad,
                    &wic_decoder
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Could not reach file: "s + path.data() ); return;
                }


                result = wic_decoder -> GetFrame( 0, &wic_frame );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Could not pull frame." ); 

                    kill_wics();

                    return;
                }


                result = renderer._wic_factory -> CreateFormatConverter( &wic_converter );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Could not create converter." ); 
                    
                    kill_wics();

                    return;
                }


                result = wic_converter -> Initialize(
                    wic_frame,
                    GUID_WICPixelFormat32bppPBGRA,
                    WICBitmapDitherTypeNone,
                    nullptr,
                    0.0,
                    WICBitmapPaletteTypeCustom
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Could not initialize converter." ); 
                    
                    kill_wics();

                    return;
                }


                result = renderer._target -> CreateBitmapFromWicBitmap(
                    wic_converter,
                    NULL,
                    _get()
                );

                if( result != S_OK ) {
                    echo( this, Echo :: FAULT, "Could not create bitmap." ); 
                    
                    kill_wics();

                    return;
                }


                uint32_t width = 0;
                uint32_t height = 0;

                wic_frame -> GetSize( &width, &height );

                _half_bw = ( _base_width = static_cast< float >( width ) ) / 2.0;
                _half_bh = ( _base_height = static_cast< float >( height ) ) / 2.0;


                if( _base_width <= 0 || _base_height <= 0 ) {
                    echo( this, Echo :: FAULT, "Dimensions ill-formed." ); 
                    
                    kill_wics();

                    return;
                }


                _sample_br = { _base_width, _base_height };


                _refresh_doffs();

                
                echo( this, Echo :: OK, "Created from: "s + path.data() );
            }

            Sprite( Ref< Renderer > renderer, Ref< Sprite > other, Echo echo = {} );

            Sprite( Ref< Sprite > other, Echo echo = {} );

        public:
            ~Sprite() {
                if( _bitmap.use_count() == 1 )
                    _bitmap -> Release();
            }

        private: 
            Shared< ID2D1Bitmap >            _bitmap        = nullptr;
            std :: string                    _path          = {};

            float                            _base_width    = 0;
            float                            _base_height   = 0;
            float                            _half_bw       = 0;
            float                            _half_bh       = 0;

            float                            _scaleX        = 1.0;
            float                            _scaleY        = 1.0;

            float                            _angelX        = 0.0;
            float                            _angelY        = 0.0;
            float                            _angelZ        = 0.0;
                        
            float                            _opac          = 1.0;

            D2D1_BITMAP_INTERPOLATION_MODE   _mode          = D2D1_BITMAP_INTERPOLATION_MODE :: D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;

            Coord< float >                   _sample_tl     = {};
            Coord< float >                   _sample_br     = {};

            Vec2                             _pin           = {};

            float                            _doffs[ 4 ]    = {};

        public:
            operator bool () const {
                return _bitmap;
            }

        private:
            ID2D1Bitmap** _get() {
                return reinterpret_cast< ID2D1Bitmap** >( &_bitmap );
            }

            void _refresh_doffs() {
                _doffs[ 0 ] = -_half_bw + _sample_tl.x - _pin.x; 
                _doffs[ 1 ] = -_half_bh + _sample_tl.y + _pin.y;
                _doffs[ 2 ] = -_half_bw + _sample_br.x - _pin.x; 
                _doffs[ 3 ] = -_half_bh + _sample_br.y + _pin.y;
            }

        public:
            enum Pin_point {
                TOP_LEFT, TOP_RIGHT, BOT_LEFT, BOT_RIGHT
            };

        public:
            std :: string_view path() const {
                return _path;
            }

        public:
            float base_width() const {
                return _base_width;
            }

            float base_height() const {
                return _base_height;
            }

            float width() const {
                return _base_width * _scaleX;
            }

            float height() const {
                return _base_height * _scaleY;
            }

        public:
            float angelX() const {
                return _angelX;
            }

            float angelY() const {
                return _angelY;
            }

            float angelZ() const {
                return _angelZ;
            }

            float scaleX() const {
                return _scaleX;
            }

            float scaleY() const {
                return _scaleY;
            }

            float scale() const {
                return scaleX();
            }

        public:
            float opac() const {
                return _opac;
            }

        public:
            Coord< float > topleft() const {
                return _sample_tl;
            }

            Coord< float > botright() const {
                return _sample_br;
            }

        public:
            Vec2 pin() const {
                return _pin;
            }

        public:
            Sprite& spinX_with( float theta ) {
                _angelX += theta;
                return *this;
            }

            Sprite& spinX_at( float theta ) {
                _angelX = theta;
                return *this;
            }

            Sprite& spinY_with( float theta ) {
                _angelY += theta;
                return *this;
            }

            Sprite& spinY_at( float theta ) {
                _angelY = theta;
                return *this;
            }

            Sprite& spinZ_with( float theta ) {
                _angelZ += theta;
                return *this;
            }

            Sprite& spinZ_at( float theta ) {
                _angelZ = theta;
                return *this;
            }

            Sprite& spin_with( float theta ) {
                return spinZ_with( theta );
            }

            Sprite& spin_at( float theta ) {
                return spinZ_at( theta );
            }

        public:
            Sprite& width_to( float value ) {
                return scaleX_at( value / base_width() );
            }

            Sprite& height_to( float value ) {
                return scaleY_at( value / base_height() );
            }

            Sprite& width_to_keep( float value ) {
                return width_to( value ).scaleY_at( scaleX() );
            }

            Sprite& height_to_keep( float value ) {
                return height_to( value ).scaleX_at( scaleY() );
            }

            Sprite& scaleX_with( float delta ) {
                _scaleX *= delta;
                return *this;
            }

            Sprite& scaleX_at( float delta ) {
                _scaleX = delta;
                return *this;
            }

            Sprite& scaleY_with( float delta ) {
                _scaleY *= delta;
                return *this;
            }

            Sprite& scaleY_at( float delta ) {
                _scaleY = delta;
                return *this;
            }

            Sprite& scale_with( float delta ) {
                scaleX_with( delta ); scaleY_with( delta );
                return *this;
            }

            Sprite& scale_at( float delta ) {
                scaleX_at( delta ); scaleY_at( delta );
                return *this;
            }

        public:
            Sprite& smooth() {
                _mode = D2D1_BITMAP_INTERPOLATION_MODE :: D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
                return *this;
            }

            Sprite& rough() {
                _mode = D2D1_BITMAP_INTERPOLATION_MODE :: D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
                return *this;
            }
       
        public:
            Sprite& opac_to( float value ) {
                _opac = value;
                return *this;
            }

        public:
            Sprite& topleft_to( Coord< float > crd ) {
                _sample_tl = crd;

                _refresh_doffs();

                return *this;
            }

            Sprite& botright_to( Coord< float > crd ) {
                _sample_br = crd;

                _refresh_doffs();

                return *this;
            }

        public:
            Sprite& pin_to( const Vec2& pin ) {
                _pin = pin;

                _refresh_doffs();

                return *this;
            }

            Sprite& pin_to( Pin_point point ) {
                switch( point ) {
                    case Pin_point :: TOP_LEFT:  _pin = { -_half_bw, _half_bh }; break;
                    case Pin_point :: TOP_RIGHT: _pin = { _half_bw,  _half_bh }; break;
                    case Pin_point :: BOT_LEFT:  _pin = { -_half_bw, -_half_bh }; break;
                    case Pin_point :: BOT_RIGHT: _pin = { _half_bw,  -_half_bh }; break;
                }

                _refresh_doffs();

                return *this;
            }
        
        public:
            Sprite& pull( Ref< Sprite > other );

            Sprite& push( Ref< Sprite > other );

            Sprite& operator << ( Ref< Sprite > other );

            Sprite& operator >> ( Ref< Sprite > other );

            Sprite& pull( const Clust2& clust ) {
                _scaleX = clust.scaleX();
                _scaleY = clust.scaleY();
                _angelZ = clust.angel();

                return *this;
            }

            Sprite& push( Clust2& clust ) {
                clust.scaleX_at( _scaleX );
                clust.scaleY_at( _scaleY );
                clust.spin_at  ( _angelZ );

                return *this;
            }

            Sprite& operator << ( const Clust2& clust ) {
                return pull( clust );
            }

            Sprite& operator >> ( Clust2& clust ) {
                return push( clust );
            }

        public:
            void render( Ref< Renderer > renderer, Coord< float > crd ) const {
                renderer.push( {
                    angel:  _angelZ,
                    scaleX: _scaleX,
                    scaleY: _scaleY
                } ).apply_around( crd );

                renderer._target -> DrawBitmap(
                    _bitmap,

                    D2D1 :: RectF(
                        crd.x + _doffs[ 0 ], 
                        crd.y + _doffs[ 1 ],
                        crd.x + _doffs[ 2 ], 
                        crd.y + _doffs[ 3 ]
                    ),

                    _opac,

                    _mode,

                    *reinterpret_cast< D2D1_RECT_F* >( 
                        const_cast< Coord< float >* >( &_sample_tl ) 
                    ) 
                );

                renderer.pop();
            }

            virtual void render( Ref< Renderer > renderer, Vec2 vec ) const override {
                render( renderer, renderer._surface -> pull_coord( vec ) );
            }

        public:/*
            static Sprite from_file( Ref< Renderer > renderer, std :: string_view path ) {
                static const char* strs[] = {
                    "path:", "angel:", "opac:", "scale:", "scaleX:", "scaleY:"
                };
                enum Attrs {
                    PATH = 0, ANGEL, OPAC, SCALE, SCALEX, SCALEY
                };
                

                std :: ifstream file( path.data() );


                Sprite spr{};

        
                File :: auto_nav( 
                    file, strs, strs + std :: size( strs ), 
                    [ & ] ( ptrdiff_t idx, std :: string& attr ) 
                -> void {
                    switch( idx ) {
                        case Attrs :: PATH: {
                            file >> attr;
                        
                            spr = Sprite{ renderer, attr }.pull( spr );

                        break; }

                        case Attrs :: ANGEL: 
                        case Attrs :: OPAC:
                        case Attrs :: SCALE:
                        case Attrs :: SCALEX:
                        case Attrs :: SCALEY: {
                            double value = 0.0; file >> value;

                            switch( idx ) {
                                case Attrs :: ANGEL:  spr.spin_at( value ); break;
                                case Attrs :: OPAC:   spr.opac_to( value ); break;
                                case Attrs :: SCALE:  spr.scale_at( value ); break;
                                case Attrs :: SCALEX: spr.scaleX_at( value ); break;
                                case Attrs :: SCALEY: spr.scaleY_at( value ); break;
                            }

                        break; }
                    }
                    
                } );

                return spr;
            }
            
            static std :: vector< Sprite > from_file_m( Ref< Renderer > renderer, std :: string_view path ) {
                static const char* strs[] = {
                    "path:", "angel:", "opac:", "scale:", "scaleX:", "scaleY:"
                };
                

                std :: vector< Sprite > sprs;

                std :: ifstream file( path.data() );


                struct Attrs {
                    std :: optional< std :: string >   path        = {};
                    std :: optional< double >          dbls[ 5 ]   = {};

                    std :: optional< double >& operator [] ( std :: size_t idx ) {
                        return dbls[ idx - 1 ];
                    }

                    enum Idx {
                        PATH = 0, ANGEL, OPAC, SCALE, SCALEX, SCALEY
                    };
                } attrs = {};


                auto push = [ & ] () -> void {
                    if( 
                        auto itr
                        =
                        std :: find_if( sprs.begin(), sprs.end(), [ &attrs ] ( Ref< Sprite > spr ) -> bool {
                            return spr._path == attrs.path.value();
                        } );

                        itr != sprs.end()
                    )
                        sprs.emplace_back( *itr );
                    else
                        sprs.emplace_back( renderer, attrs.path.value() );
                    

                    sprs.back().spin_at    ( attrs[ Attrs :: ANGEL ] .value_or( 0.0 ) )
                               .opac_to    ( attrs[ Attrs :: OPAC ]  .value_or( 1.0 ) )
                               .scaleX_with( attrs[ Attrs :: SCALEX ].value_or( 1.0 ) )
                               .scaleY_with( attrs[ Attrs :: SCALEY ].value_or( 1.0 ) )
                               .scale_with ( attrs[ Attrs :: SCALE ] .value_or( 1.0 ) );


                    attrs.path.reset();

                    for( auto& dbl : attrs.dbls )
                        dbl.reset();
                };

        
                File :: auto_nav( 
                    file, strs, strs + std :: size( strs ), 
                    [ & ] ( ptrdiff_t idx, std :: string& attr ) 
                -> void {
                    switch( idx ) {
                        case Attrs :: PATH: {
                            if( attrs.path.has_value() )
                                push();

                            file >> attr;

                            attrs.path = attr;

                        break; }

                        case Attrs :: ANGEL:
                        case Attrs :: OPAC:
                        case Attrs :: SCALE:
                        case Attrs :: SCALEX:
                        case Attrs :: SCALEY: {
                            if( attrs[ idx ].has_value() )
                                push();

                            double value = 0.0;

                            file >> value;

                            attrs[ idx ] = value;

                        break; }
                    }
                    
                } );

                push();

                return sprs;
            }
        */

        };



        


        /*
        class Core :: _Sprite_chain {

        };



        template< bool is_strong = true >
        class Sprite_chain : public Shell_of< Core :: _Sprite_chain, Sprite_chain, is_strong > {
        public:
            static std :: vector< Sprite<> > from_file( View< Renderer > renderer, std :: string_view path ) {
                static const char* strs[] = {
                    "path:", "angel:", "opac:", "scale:", "scaleX:", "scaleY:"
                };
                

                std :: vector< Sprite<> > sprs;

                std :: ifstream file( path.data() );


                struct Attrs {
                    std :: optional< std :: string >   path        = {};
                    std :: optional< double >          dbls[ 5 ]   = {};

                    std :: optional< double >& operator [] ( std :: size_t idx ) {
                        return dbls[ idx - 1 ];
                    }

                    enum ATTRS_ACCESS_INDEX {
                        PATH = 0, ANGEL, OPAC, SCALE, SCALEX, SCALEY
                    };
                } attrs = {};


                auto push = [ & ] () -> void {
                    if( 
                        auto itr
                        =
                        std :: find_if( sprs.begin(), sprs.end(), [ &attrs ] ( View< Sprite > spr ) -> bool {
                            return spr -> _path == attrs.path.value();
                        } );

                        itr != sprs.end()
                    )
                        sprs.push_back( make< Sprite >( *itr ) );
                    else
                        sprs.push_back( make< Sprite >( renderer, attrs.path.value() ) );
                    

                    sprs.back().spin_at    ( attrs[ Attrs :: ANGEL ] .value_or( 0.0 ) )
                               .opac_to    ( attrs[ Attrs :: OPAC ]  .value_or( 1.0 ) )
                               .scaleX_with( attrs[ Attrs :: SCALEX ].value_or( 1.0 ) )
                               .scaleY_with( attrs[ Attrs :: SCALEY ].value_or( 1.0 ) )
                               .scale_with ( attrs[ Attrs :: SCALE ] .value_or( 1.0 ) );


                    attrs.path.reset();

                    for( auto& dbl : attrs.dbls )
                        dbl.reset();
                };

        
                File :: build( 
                    file, strs, strs + std :: size( strs ), 
                    [ & ] ( ptrdiff_t idx, std :: string& attr ) 
                -> void {
                    switch( idx ) {
                        case Attrs :: PATH: {
                            if( attrs.path.has_value() )
                                push();

                            file >> attr;

                            attrs.path = attr;

                        break; }

                        case Attrs :: ANGEL:
                        case Attrs :: OPAC:
                        case Attrs :: SCALE:
                        case Attrs :: SCALEX:
                        case Attrs :: SCALEY: {
                            if( attrs[ idx ].has_value() )
                                push();

                            double value = 0.0;

                            file >> value;

                            attrs[ idx ] = value;

                        break; }
                    }
                    
                } );

                push(); 

                return sprs;
            }

        };*/

    #pragma endregion Sprite



    Renderer& Renderer :: line(
        Coord< float > c1, Coord< float > c2,
        Ref< Is_brush auto > brush
    ) { 
        _target -> DrawLine(
            c1, c2,
            brush._brush,
            brush._width
        );

        return *this;
    }

    Renderer& Renderer :: line(
        Vec2 v1, Vec2 v2,
        Ref< Is_brush auto > brush
    ) {
        return line(
            _surface -> pull_coord( v1 ),
            _surface -> pull_coord( v2 ),
            brush
        );
    }

    Renderer& Renderer :: fill( const Chroma& chroma = {} ) {
        _target -> Clear( chroma );

        return *this;
    }



    Sprite :: Sprite( Ref< Renderer > renderer, Ref< Sprite > other, Echo echo )
        : _path       { other._path },
          _base_width { other._base_width },
          _half_bw    { other._half_bw },
          _base_height{ other._base_height },
          _half_bh    { other._half_bh },
          _sample_br  { Coord< float >{ other._base_width, other._base_height } }
    {
        using namespace std :: string_literals;

        HRESULT result;


        float dpiX, dpiY;
        other._bitmap -> GetDpi( &dpiX, &dpiY ); 

        result = renderer._target -> CreateBitmap(
            D2D1_SIZE_U{ other._base_width, other._base_height },
            D2D1 :: BitmapProperties(
                other._bitmap -> GetPixelFormat(),
                dpiX, dpiY
            ),
            _get()
        );

        if( result != S_OK ) {
            echo( this, Echo :: FAULT, "Could not create bitmap." ); return;
        }

    
        result = _bitmap -> CopyFromBitmap(
            nullptr,
            other._bitmap,
            nullptr
        );

        if( result != S_OK ) {
            echo( this, Echo :: FAULT, "Could not copy bitmap." ); return;
        }


        _refresh_doffs();

        echo( this, Echo :: OK, "Created as copy from: "s + other._path );
    }

    Sprite :: Sprite( Ref< Sprite > other, Echo echo ) 
        : _path       { other._path },
          _bitmap     { other._bitmap },
          _base_width { other._base_width },
          _half_bw    { other._half_bw },
          _base_height{ other._base_height },
          _half_bh    { other._half_bh },
          _sample_br  { Coord< float >{ other._base_width, other._base_height } }
    {   
        using namespace std :: string_literals;

        _refresh_doffs();

        echo( this, Echo :: OK, "Created as borrower from: "s + other._path );
    };

    Sprite& Sprite :: pull( Ref< Sprite > other ) {
        _scaleX    = other._scaleX;
        _scaleY    = other._scaleY;
        _angelX    = other._angelX;
        _angelY    = other._angelY;
        _angelZ    = other._angelZ;
        _opac      = other._opac;
        _mode      = other._mode;
        _sample_tl = other._sample_tl;
        _sample_br = other._sample_br;

        return *this;
    }

    Sprite& Sprite :: push( Ref< Sprite > other ) {
        return other.pull( *this );
    }

    Sprite& Sprite :: operator << ( Ref< Sprite > other ) {
        return pull( other );
    }

    Sprite& Sprite :: operator >> ( Ref< Sprite > other ) {
        return push( other );
    }



    void Vec2 :: render(
        Ref< Renderer > renderer ,
        Ref< Is_brush auto > brush
    ) const {
        renderer.line( Vec2 :: O(), *this, brush );
    }


    void Ray2 :: render(
        Ref< Renderer > renderer ,
        Ref< Is_brush auto > brush
    ) const {
        renderer.line( origin, end(), brush );
    }


    void Clust2 :: render(
        Ref< Renderer > renderer ,
        Ref< Is_brush auto > brush
    ) const {
        for( std :: size_t idx = 0; idx < vrtx_count(); ++idx )
            renderer.line( ( *this )( idx ), ( *this )( ( idx + 1 ) % vrtx_count() ), brush );
    }


#pragma endregion Graphics



#pragma region Audio

    class Sound : public Has_op_ptr< Sound > {
    public:
        inline static const char*   name   = _ENGINE_CLASS_NAME( "Sound" );

    public:
        friend class Audio;

    public:
        typedef   std :: function< double( double, size_t ) >   Filter;

    public:
        Sound() = default;

        Sound( Ptr< Audio > audio, std :: string_view path, Echo echo = {} );

        Sound( std :: string_view path, Echo echo = {} )
            : Sound( nullptr, path )
        {}

        Sound( Ref< const Sound > other ) = default;


        Sound( Sound&& other ) = delete;


        ~Sound() {
            stop();
        }

    private:
        Ptr< Audio >             _audio              = nullptr;

        Shared< double[] >       _stream             = nullptr;

        std :: list< double >    _needles            = {};
    
        size_t                   _sample_rate        = 0;
        size_t                   _bits_per_sample    = 0;
        size_t                   _sample_count       = 0;
        size_t                   _channel_count      = 0;

        bool                     _loop               = false;
        bool                     _pause              = false;
        bool                     _mute               = false;

        Filter                   _filter             = nullptr;
        double                   _volume             = 1.0;
        double                   _velocity           = 1.0;

    public:
        Sound& lock_on( Ptr< Audio > audio ) {
            _audio = std :: move( audio );

            return *this;
        }

    public:
        bool is_locked() const {
            return _audio != nullptr;
        }

        bool has_stream() const {
            return _stream != nullptr;
        }

        operator bool () const {
            return is_locked() && has_stream();
        }

    public:
        bool is_playing() const;

        Sound& play();

        Sound& stop();

    public:
        Sound& loop() {
            _loop = true;

            return *this;
        }

        Sound& unloop() {
            _loop = false;

            return *this;
        }

        Sound& swap_loop() {
            _loop ^= true;

            return *this;
        }

        bool is_looping() const {
            return _loop;
        }


        Sound& pause() {
            _pause = true;

            return *this;
        }

        Sound& resume() {
            _pause = false;

            return *this;
        }

        Sound& swap_pause() {
            _pause ^= true;

            return *this;
        }

        bool is_paused() const {
            return _pause;
        }


        Sound& mute() {
            _mute = true;

            return *this;
        }

        Sound& unmute() {
            _mute = false;

            return *this;
        }

        Sound& swap_mute() {
            _mute ^= true;

            return *this;
        }

        bool is_muted() {
            return _mute;
        }


        Sound& volume_to( double vlm ) {
            _volume = vlm;

            return *this;
        }

        double volume() const {
            return _volume;
        }


        Sound& filter_to( Filter flt ) {
            _filter = flt;

            return *this;
        }

        Filter filter() const {
            return _filter;
        }


        Sound& velocity_to( double vlc ) {
            _velocity = vlc;

            return *this;
        }

        double velocity() const {
            return _velocity;
        }

    public:
        size_t sample_rate() const {
            return _sample_rate;
        }

        size_t channel_count() const {
            return _channel_count;
        }

        size_t sample_count() const {
            return _sample_count;
        }

        double duration() const {
            return static_cast< double >( _sample_count ) / _sample_rate;
        }

    };



    class Audio : public Has_op_ptr< Audio > {
    public:
        inline static const char*   name   = _ENGINE_CLASS_NAME( "Audio" );

    private:
        friend class Sound;

    public:
        Audio() = default;

        Audio(  
            std :: string_view device,
            size_t             sample_rate        = 48000, 
            size_t             channel_count      = 1,
            size_t             block_count        = 16, 
            size_t             block_sample_count = 256,
            Echo               echo               = {}
        ) 
            : _device            { device.data() },
              _sample_rate       { sample_rate }, 
              _channel_count     { channel_count },
              _block_count       { block_count }, 
              _block_sample_count{ block_sample_count },
              _block_current     { 0 },
              _block_memory      { nullptr },
              _wave_headers      { nullptr },
              _free_block_count  { block_count }
        {
            uint32_t dev_idx = 0;

            auto devs = devices();

            for( auto& dev : devs ) {
                if( dev == _device ) break;

                ++dev_idx;
            }

            if( dev_idx == devs.size() ) {
                echo( this, Echo :: FAULT, "Device does not exist." ); return;
            }


            WAVEFORMATEX wave_format;

            wave_format.wFormatTag      = WAVE_FORMAT_PCM;
            wave_format.nSamplesPerSec  = _sample_rate;
            wave_format.wBitsPerSample  = sizeof( int ) * 8;
            wave_format.nChannels       = _channel_count;
            wave_format.nBlockAlign     = ( wave_format.wBitsPerSample / 8 ) * wave_format.nChannels;
            wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
            wave_format.cbSize          = 0;


            auto result = waveOutOpen( 
                &_wave_out, dev_idx, &wave_format, 
                ( DWORD_PTR ) event_proc_router, 
                ( DWORD_PTR ) this, 
                CALLBACK_FUNCTION 
            );
            
            if( result != S_OK ) { 
                echo( this, Echo :: FAULT, "Wave open link failed." ); return;
            }


            _block_memory = new int[ _block_count * _block_sample_count ];

            if( !_block_memory ) {
                echo( this, Echo :: FAULT, "Block alloc failed." ); return;
            }

            std :: fill_n( _block_memory.get(), _block_count * _block_sample_count, 0 );


            _wave_headers = new WAVEHDR[ _block_count ];

            if( !_wave_headers ) { 
                echo( this, Echo :: FAULT, "Wave headers alloc failed." ); return;
            }

            std :: fill_n( ( char* ) _wave_headers.get(), sizeof( WAVEHDR ) * _block_count, 0 );


            for( size_t n = 0; n < _block_count; ++n ) { 
                _wave_headers[ n ].dwBufferLength = sizeof( int ) * _block_sample_count;
                _wave_headers[ n ].lpData = ( char* ) ( _block_memory + ( n * _block_sample_count ) );
            }


            _powered = true;

            _thread = std :: thread( _main, this );

            if( !_thread.joinable() ) {
                echo( this, Echo :: FAULT, "Thread launch failed." ); return;
            }

            std :: unique_lock< std :: mutex > lock{ _mtx };
            _cnd_var.notify_one();
            
            echo( this, Echo :: OK, "Created." );
        }


        Audio( const Audio& other ) = delete;

        Audio( Audio&& other ) = delete;


        ~Audio() {
            _powered = false;

            _cnd_var.notify_one();

            if( _thread.joinable() )
                _thread.join();

            waveOutReset( _wave_out );
            waveOutClose( _wave_out );
        }

    private:
        volatile bool                 _powered              = false;

        size_t                        _sample_rate          = 0;
        size_t                        _channel_count        = 0;
        size_t                        _block_count          = 0;
        size_t                        _block_sample_count   = 0;
        size_t                        _block_current        = 0;
        Unique< int[] >               _block_memory         = nullptr;

        Unique< WAVEHDR[] >           _wave_headers         = nullptr;
        HWAVEOUT                      _wave_out             = nullptr;
        std :: string                 _device               = {};

        std :: thread                 _thread               = {};

        std :: atomic< size_t >       _free_block_count     = 0;
        std :: condition_variable     _cnd_var              = {};
        std :: mutex                  _mtx                  = {};

        std :: list< Ptr< Sound > >   _sounds               = {};

        bool                          _pause                = false;
        bool                          _mute                 = false;
    
        Sound :: Filter               _filter               = nullptr;
        double                        _volume               = 1.0;
        double                        _velocity             = 1.0;

    private:
        void _main() {
            constexpr double max_sample = static_cast< double >( 
                std :: numeric_limits< int > :: max()
             );


            auto clip = [] ( double amp ) -> double {
                return amp >= 0.0 ? std :: min( amp, 1.0 ) : std :: max( amp, -1.0 );
            };

            auto sample = [ this ] ( size_t channel ) -> double { 
                double amp = 0.0;

                if( _pause ) return amp;

                for( Ptr< Sound > snd : _sounds ) {
                    if( snd -> _pause ) continue;

                    snd -> _needles.remove_if( [ this, &snd, &amp, &channel ] ( Ref< double > at ) {
                        if( snd -> _filter )
                            amp += snd -> _filter( 
                                    snd -> _stream[ static_cast< size_t >( at ) ], 
                                    channel 
                                )
                                *
                                snd -> _volume * !snd -> _mute
                                *
                                _volume * !_mute;
                        else
                            amp += snd -> _stream[ static_cast< size_t >( at ) ]
                                *
                                snd -> _volume * !snd -> _mute
                                *
                                _volume * !_mute;


                        at += snd -> _velocity * _velocity;

                        if( at >= snd -> _sample_count ) { 
                            at = 0; 

                            return !snd -> _loop; 
                        }

                        return false;
                    } );
                }

                if( _filter )
                    return _filter( amp, channel );
                else
                    return amp;
            };


            while( _powered ) {
                if( _free_block_count == 0 ) {
                    std :: unique_lock< std :: mutex > lock{ _mtx };

                    _cnd_var.wait( lock );
                }

                --_free_block_count;


                if( _wave_headers[ _block_current ].dwFlags & WHDR_PREPARED )
                    waveOutUnprepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );


                _sounds.remove_if( [] ( Ptr< Sound > snd ) {
                    return snd -> _needles.empty();
                } );


                size_t current_block = _block_current * _block_sample_count;
                
                for( size_t n = 0; n < _block_sample_count; n += _channel_count )
                    for( size_t c = 0; c < _channel_count; ++c )
                        _block_memory[ current_block + n + c ] = static_cast< int >( clip( sample( c ) ) * max_sample );
                

                waveOutPrepareHeader( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );
                waveOutWrite( _wave_out, &_wave_headers[ _block_current ], sizeof( WAVEHDR ) );


                ++_block_current;
                _block_current %= _block_count; 
            }
            
        }

    private:
        static void CALLBACK event_proc_router( HWAVEOUT hwo, UINT event, DWORD_PTR instance, DWORD w_param, DWORD l_param ) {
            reinterpret_cast< Audio* >( instance ) -> event_proc( hwo, event, w_param, l_param);
        }

        void event_proc( HWAVEOUT hwo, UINT event, DWORD w_param, DWORD l_param ) {
            switch( event ) {
                case WOM_OPEN: {
                    
                break; }

                case WOM_DONE: {
                    ++_free_block_count;

                    std :: unique_lock< std :: mutex > lock{ _mtx };
                    _cnd_var.notify_one();
                break; }

                case WOM_CLOSE: {
                    /* Here were the uniques deleted[] */
                break; }
            }
        }

    public:
        static std :: vector< std :: string > devices() {
            WAVEOUTCAPS woc;

            std :: vector< std :: string > devs;

            for( int n = 0; n < waveOutGetNumDevs(); ++n ) {
                if( waveOutGetDevCaps( n, &woc, sizeof( WAVEOUTCAPS ) ) != S_OK ) continue;
                
                devs.emplace_back( woc.szPname );
            }

            return devs;
        }

    public:
        Audio& pause() {
            _pause = true;

            return *this;
        }

        Audio& resume() {
            _pause = false;

            return *this;
        }

        Audio& swap_pause() {
            _pause ^= true;

            return *this;
        }

        bool is_paused() const {
            return _pause;
        }


        Audio& mute() {
            _mute = true;

            return *this;
        }

        Audio& unmute() {
            _mute = false;

            return *this;
        }

        Audio& swap_mute() {
            _mute ^= true;

            return *this;
        }

        bool is_muted() {
            return _mute;
        }


        Audio& volume_to( double vlm ) {
            _volume = vlm;

            return *this;
        }

        double volume() const {
            return _volume;
        }


        Audio& filter_to( Sound :: Filter flt ) {
            _filter = flt;

            return *this;
        }

        Sound :: Filter filter() const {
            return _filter;
        }


        Audio& velocity_to( double vlc ) {
            _velocity = vlc;

            return *this;
        }

        double velocity() const {
            return _velocity;
        }


    public:
        std :: string_view device() const {
            return _device;
        }

        Audio& device_to( std :: string_view dev ) {
            return *this;
        }

    };



    Sound :: Sound( Ptr< Audio > audio, std :: string_view path, Echo echo = {} ) 
        : _audio{ std :: move( audio ) }
    {
        using namespace std :: string_literals;


        std :: ifstream file{ path.data(), std :: ios_base :: binary };

        if( !file ) {
            echo( this, Echo :: FAULT, "File open failed: "s + path.data() ); return;
        }


        size_t file_size = File :: size( file );

        Unique< char[] > file_stream{ new char[ file_size ] };

        if( file_stream == nullptr ) {
            echo( this, Echo :: FAULT, "File stream alloc failed." ); return;
        }


        file.read( file_stream, file_size );


        _sample_rate = Bytes :: as< unsigned int >( file_stream + 24, 4, Bytes :: LITTLE );

        if( _sample_rate != _audio -> _sample_rate ) 
            echo( this, Echo :: WARNING, "Sample rate does not match with locked on audio's." );


        _bits_per_sample = Bytes :: as< unsigned short >( file_stream + 34, 2, Bytes :: LITTLE );

        size_t bytes_per_sample = _bits_per_sample / 8;

        _sample_count = Bytes :: as< size_t >( file_stream + 40, 4, Bytes :: LITTLE ) 
                        / 
                        bytes_per_sample;


        _stream = new double[ _sample_count ];

        if( _stream == nullptr ) {
            echo( this, Echo :: FAULT, "Sound stream alloc failed." ); return;
        }


        double max_sample = static_cast< double >( 1 << ( _bits_per_sample - 1 ) );

        for( size_t n = 0; n < _sample_count; ++n )
            _stream[ n ] = static_cast< double >( 
                                Bytes :: as< int >( file_stream + 44 + n * bytes_per_sample, bytes_per_sample, Bytes :: LITTLE )
                            ) / max_sample;
        

        _channel_count = Bytes :: as< unsigned short >( file_stream + 22, 2, Bytes :: LITTLE );

        if( 
            _audio -> _channel_count 
            != 
            _channel_count
        )
            echo( this, Echo :: WARNING, "Channel count does not match with locked on audio's." );


        echo( this, Echo :: OK, "Created from: "s + path.data() );
    }

    bool Sound :: is_playing() const {
        return std :: find( _audio -> _sounds.begin(), _audio -> _sounds.end(), this )
               !=
               _audio -> _sounds.end();
    }

    Sound& Sound :: play() {
        _needles.push_back( 0 );

        if( !is_playing() )
            _audio -> _sounds.push_back( this );

        return *this;
    }

    Sound& Sound :: stop() {
        _needles.clear();

        return *this;
    }

#pragma endregion Audio



};
