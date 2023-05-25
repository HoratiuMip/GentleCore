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

    #define _ENGINE_NAMESPACE Gtl

    #if defined( GTL_ECHO )
        #define _ENGINE_ECHO
    #endif

#pragma endregion Defines



namespace _ENGINE_NAMESPACE {



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

                std :: cout << _type_name( type ) << " ] "_normal << _type_fill( type ) << ""_special;

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



#pragma region Pointers



    template< template< typename T > typename P, typename T, typename I >
    class _Smart_ptr_extended : public P< T > {
    public:
        using P< T > :: P;

        using P< T > :: operator =;

    public:
        static inline constexpr bool   is_array   = std :: is_array_v< T >;

    public:
        typedef   std :: conditional_t< is_array, std :: decay_t< T >, T* >   T_ptr;

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
        operator std :: enable_if_t< isnt_array, T& > () {
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
    class Unique_ptr : public _Smart_ptr_extended< std :: unique_ptr, T, Unique_ptr< T > > {
    public:
        typedef   _Smart_ptr_extended< std :: unique_ptr, T, Unique_ptr< T > >   _Base;

        using _Base :: _Base;

        using _Base :: operator =;

    };


    template< typename T >
    class Shared_ptr : public _Smart_ptr_extended< std :: shared_ptr, T, Shared_ptr< T > > {
    public:
        typedef   _Smart_ptr_extended< std :: shared_ptr, T, Shared_ptr< T > >   _Base;

        using _Base :: _Base;

        using _Base :: operator =;

    };


    template< typename T >
    using Ref = T&;

    template< typename T >
    using Ptr = T*;


    template< typename I >
    class Has_op_ptr {
    public:
        operator I* () {
            return reinterpret_cast< I* >( this );
        }

    };



#pragma endregion Pointers



#pragma region Utility



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
        static void build( 
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





#pragma endregion Utility



#pragma region Audio



class Audio;

    

class Sound : public Has_op_ptr< Sound > {
public:
    inline static const char*   name   = "Sound";

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

    ~Sound() {
        stop();
    }

private:
    Ptr< Audio >             _audio              = nullptr;

    Shared_ptr< double[] >   _stream             = nullptr;

    std :: list< size_t >    _needles            = {};
 
    size_t                   _sample_rate        = 0;
    size_t                   _bits_per_sample    = 0;
    size_t                   _sample_count       = 0;

    bool                     _loop               = false;
    bool                     _pause              = false;
    bool                     _mute               = false;

    Filter                   _filter             = nullptr;
    double                   _volume             = 1.0;

public:
    Sound& lock_on( Ptr< Audio > audio ) {
        _audio = audio;

        return *this;
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

};



class Audio : public Has_op_ptr< Audio > {
public:
    inline static const char*   name   = "Audio";

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
          _block_memory      { NULL },
          _wave_headers      { NULL },
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

    ~Audio() {
        _powered = false;

        _cnd_var.notify_one();

        if( _thread.joinable() )
            _thread.join();

        waveOutReset( _wave_out );
        waveOutClose( _wave_out );
    }

private:
    bool                          _powered              = false;

    size_t                        _sample_rate          = 0;
    size_t                        _channel_count        = 0;
    size_t                        _block_count          = 0;
    size_t                        _block_sample_count   = 0;
    size_t                        _block_current        = 0;
    Unique_ptr< int[] >           _block_memory         = nullptr;

    Unique_ptr< WAVEHDR[] >       _wave_headers         = nullptr;
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

private:
    void _main() {
        constexpr double max_sample = static_cast< double >( ( -1 ) ^ ( 1 << 31 ) );


        auto clip = [] ( double amp ) -> double {
            return amp >= 0.0 ? std :: min( amp, 1.0 ) : std :: max( amp, -1.0 );
        };

        auto sample = [ this ] ( size_t channel ) -> double { 
            double amp = 0.0;

            if( _pause ) return amp;

            for( Ptr< Sound > snd : _sounds ) {
                if( snd -> _pause ) continue;

                snd -> _needles.remove_if( [ this, &snd, &amp, &channel ] ( Ref< size_t > at ) {
                    if( snd -> _filter )
                        amp += snd -> _filter( snd -> _stream[ at ], channel )
                               *
                               snd -> _volume * !snd -> _mute
                               *
                               _volume * !_mute;
                    else
                        amp += snd -> _stream[ at ]
                               *
                               snd -> _volume * !snd -> _mute
                               *
                               _volume * !_mute;


                    if( ++at >= snd -> _sample_count ) { 
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


    std :: string_view device() const {
        return _device;
    }

    Audio& device_to( std :: string_view dev ) {
        return *this;
    }

};



Sound :: Sound( Ptr< Audio > audio, std :: string_view path, Echo echo = {} ) 
    : _audio{ audio }
{
    using namespace std :: string_literals;


    std :: ifstream file{ path.data(), std :: ios_base :: binary };

    if( !file ) {
        echo( this, Echo :: FAULT, "File open failed: "s + path.data() ); return;
    }


    size_t file_size = File :: size( file );

    Unique_ptr< char[] > file_stream{ new char[ file_size ] };

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
    

    if( 
        _audio -> _channel_count 
        != 
        Bytes :: as< unsigned short >( file_stream + 22, 2, Bytes :: LITTLE ) 
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
