/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2002-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
// config.cc author Josh Rosenbaum <jorosenba@cisco.com>

#include <sstream>
#include <vector>

#include "conversion_state.h"
#include "converter.h"
#include "snort2lua_util.h"

namespace {

class StreamTcp : public ConversionState
{
public:
    StreamTcp(Converter* cv)  : ConversionState(cv) {};
    virtual ~StreamTcp() {};
    virtual bool convert(std::stringstream& data_stream);

private:
    bool parse_small_segments(std::stringstream& data_stream);
    bool parse_ports(std::stringstream& data_stream);
};

} // namespace

bool StreamTcp::parse_small_segments(std::stringstream& data_stream)
{
    std::string s_val;
    int i_val;

    if (!(data_stream >> i_val))
        return false;

    converter->open_table("small_segments");
    converter->add_option_to_table("count", i_val);
    converter->close_table();

    if (!(data_stream >> s_val))
        return false;

    if (!s_val.compare(",") || s_val.compare("bytes"))
        return false;

    if(!(data_stream >> i_val))
        return false;

    converter->open_table("small_segments");
    converter->add_option_to_table("maximum_size", i_val);
    converter->close_table();


    if (!(data_stream >> s_val))
        return false;

    // if the next string is either a comma, end of command
    if (!s_val.compare(","))
        return true;

    // otherwise the next argument MUST be ignore_ports
    if (s_val.compare("ignore_ports"))
        return false;


    converter->open_table("small_segments");

    while(data_stream >> s_val && (s_val.back() != ','))
        converter->add_list_to_table("ignore_ports", s_val);

    if (!s_val.empty())
    {
        s_val.pop_back();
        converter->add_list_to_table("ignore_ports", s_val);
    }

    converter->close_table();
    return true;
}


bool StreamTcp::parse_ports(std::stringstream& data_stream)
{
    std::string s_val;
    std::string opt_name;
    bool retval = true;

    if(!(data_stream >> opt_name))
        return false;

    if( !opt_name.compare("client"))
    {
        converter->add_deprecated_comment("port client", "client_ports");
        opt_name = "client_ports";
    }
    else if( !opt_name.compare("server"))
    {
        converter->add_deprecated_comment("port server", "server_ports");
        opt_name = "server_ports";
    }
    else if( !opt_name.compare("both"))
    {
        converter->add_deprecated_comment("port both", "both_ports");
        opt_name = "both_ports";
    }

    else
        return false;

    while(data_stream >> s_val && (s_val.back() != ','))
        retval = converter->add_list_to_table(opt_name, s_val) && retval;

    if (!s_val.empty())
    {
        s_val.pop_back();
        converter->add_list_to_table(opt_name, s_val);
    }

    return retval;
}


bool StreamTcp::convert(std::stringstream& data_stream)
{
    std::string keyword;
    bool retval = true;

    converter->open_table("stream_tcp");

    while(data_stream >> keyword)
    {
        bool tmpval = true;

        if(keyword.back() == ',')
            keyword.pop_back();
        
        if(keyword.empty())
            continue;

        if(!keyword.compare("policy"))
            tmpval = parse_string_option("policy", data_stream);

        else if(!keyword.compare("overlap_limit"))
            tmpval = parse_int_option("overlap_limit", data_stream);

        else if(!keyword.compare("max_window"))
            tmpval = parse_int_option("max_window", data_stream);

        else if(!keyword.compare("require_3whs"))
            tmpval = parse_int_option("require_3whs", data_stream);

        else if(!keyword.compare("small_segments"))
            tmpval = parse_small_segments(data_stream);

        else if(!keyword.compare("ignore_any_rules"))
            tmpval = converter->add_option_to_table("ignore_any_rules", true);

        else if(!keyword.compare("ports"))
            tmpval = parse_ports(data_stream);

        else if(!keyword.compare("detect_anomalies"))
            converter->add_deprecated_comment("detect_anomalies");

        else if(!keyword.compare("dont_store_large_packets"))
            converter->add_deprecated_comment("dont_store_large_packets");

        else if(!keyword.compare("check_session_hijacking"))
            converter->add_deprecated_comment("check_session_hijacking");

        else if(!keyword.compare("bind_to"))
        {
            converter->add_deprecated_comment("bind_to", "bindings");
            if(!(data_stream >> keyword))
                tmpval = false;
        }

        else if(!keyword.compare("dont_reassemble_async"))
        {
            converter->add_deprecated_comment("dont_reassemble_async", "reassemble_async");
            tmpval = converter->add_option_to_table("reassemble_async", false);
        }

        else if(!keyword.compare("use_static_footprint_sizes"))
        {
            converter->add_deprecated_comment("footprint", "use_static_footprint_sizes");
            tmpval = converter->add_option_to_table("footprint", true);
        }

        else if(!keyword.compare("timeout"))
        {
            converter->add_deprecated_comment("timeout", "session_timeout");
            tmpval = parse_int_option("session_timeout", data_stream);
        }

        else if(!keyword.compare("max_queued_segs"))
        {
            converter->add_deprecated_comment("max_queued_segs", "queue_limit.max_segments");
            converter->open_table("queue_limit");
            tmpval = parse_int_option("max_segments", data_stream);
            converter->close_table();
        }

        else if(!keyword.compare("max_queued_bytes"))
        {
            converter->add_deprecated_comment("max_queued_bytes", "queue_limit.max_bytes");
            converter->open_table("queue_limit");
            tmpval = parse_int_option("max_bytes", data_stream);
            converter->close_table();
        }

        else
            tmpval = false;

        if (retval)
            retval = tmpval;
    }

    return retval;    
}

/**************************
 *******  A P I ***********
 **************************/

static ConversionState* ctor(Converter* cv)
{
    return new StreamTcp(cv);
}

static const ConvertMap preprocessor_stream_tcp = 
{
    "stream5_tcp",
    ctor,
};

const ConvertMap* stream_tcp_map = &preprocessor_stream_tcp;