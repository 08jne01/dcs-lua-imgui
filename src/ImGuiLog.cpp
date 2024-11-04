#include "ImGuiLog.h"

using namespace utils;

void Log::Draw( const char* title, bool& open )
{
    if ( ! ImGui::Begin( title, &open ) )
    {
        ImGui::End();
        return;
    }

    if ( ImGui::BeginPopup( "Options" ) )
    {
        
        ImGui::EndPopup();
    }

    if ( ImGui::Button( "Clear" ) )
        Clear();

    ImGui::SameLine();
    ImGui::Checkbox( "Auto Scroll", &auto_scroll );
    // DISABLE FILTER DEADLOCK IN RENDER CODE
    //ImGui::SameLine();
    //filter.Draw( "Filter", -100.0f );

    ImGui::Separator();

    if ( ImGui::BeginChild( "scrolling", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar ) )
    {
        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

        // DISABLE FILTER DEADLOCK IN RENDER CODE
        if constexpr ( false ) // filter.IsActive()
        {
            for ( int i = 0; i < line_locations.size(); i++ )
            {

                const char* line_start = LineStart( i );
                const char* line_end = LineEnd( i );
                if ( filter.PassFilter( line_start, line_end ) )
                    ImGui::TextUnformatted( line_start, line_end );
            }
        }
        else
        {
           
            ImGui::TextUnformatted( buffer.begin(), buffer.end() );
            /*ImGuiListClipper clipper;
            clipper.Begin( line_locations.size() );
            while ( clipper.Step() )
            {
                for ( int i = clipper.DisplayStart; i < clipper.DisplayStart; i++ )
                {
                    
                }
            }

            clipper.End();*/
        }


        ImGui::PopStyleVar();

        if ( auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() )
            ImGui::SetScrollHereY( 1.0f );
    }

    ImGui::EndChild();
    ImGui::End();
}

const char* Log::LineStart( int line_number )
{
    const char* line_start = buffer.begin() + line_locations[line_number];
    return line_start;
}

const char* Log::LineEnd( int line_number )
{
    const char* line_end = buffer.end();

    if ( line_number + 1 < line_locations.size() )
    {
        // Start of Next Line - 1
        line_end = buffer.begin() + line_locations[line_number + 1] - 1;
    }

    return line_end;
}