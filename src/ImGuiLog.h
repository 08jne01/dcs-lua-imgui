#pragma once
#include <imgui.h>
#include <memory>
#include <vector>

namespace utils
{
    class Log
    {
    public:

        Log()
        {
            Clear();
        }


        void Draw( const char* title, bool& open );

        template<class...Args>
        void Add( const char* fmt, Args&&...args )
        {

            const int old_size = buffer.size();
            buffer.appendf( fmt, args... );

            for ( int i = old_size; i < buffer.size(); i++ )
            {
                if ( buffer[i] == '\n' )
                    line_locations.push_back( i + 1 );
            }
        }

    private:

        void Clear()
        {
            buffer.clear();
            line_locations.clear();
            line_locations.push_back( 0 );
        }

        const char* LineStart( int line_number );
        const char* LineEnd( int line_number );

        ImGuiTextBuffer buffer; // complete text buffer
        ImGuiTextFilter filter; // filter text
        ImVector<int> line_locations; // indices to the starts of lines
        bool auto_scroll = true;
    };
}