//
//  main.cpp
//  brandon_3_11_2018
//
//  Created by Mr. Penguin on 3/11/18.
//  Copyright © 2018 Mr. Penguin. All rights reserved.
//
#include "Header.h"

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init(); //curl object
    CURLcode res;
    string s1;
    int i = 0;
    Document d;
    d.SetObject();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:80/api/newdeveloper");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //only for https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); //only for https
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlwrite_func_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s1);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return(1);
        }
    }
    if (s1 != "")
    {
        const char* json = s1.c_str();
        ParseResult ok = d.Parse(json);
        if (!ok) {
            fprintf(stderr, "JSON parse error: %u (%lu)", ok.Code(), ok.Offset());
            exit(EXIT_FAILURE);
        }
    }
    int n_lights = d["lights"].MemberCount();
    const Value& b = d["lights"];
    vector<HueLight> lights(n_lights);
    for (Value::MemberIterator itr_outter = d["groups"].MemberBegin(); itr_outter !=d["groups"].MemberEnd(); ++itr_outter)
    {
        const Value& a = d["groups"][itr_outter->name.GetString()]["lights"];
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        writer.StartArray();
        for (Value::ConstMemberIterator itr_inner = b.MemberBegin(); itr_inner != b.MemberEnd(); ++itr_inner)
        {
            const Value& c = a[i];
            const Value& e = b[c];
            assert(e.HasMember("name"));
            assert(e["name"].IsString());
            lights[i].name = e["name"].GetString();
            assert(d.HasMember("lights"));
            lights[i].id = c.GetString();
            assert(e.HasMember("state"));
            assert(e["state"].HasMember("on"));
            assert(e["state"]["on"].IsBool());
            lights[i].on = e["state"]["on"].GetBool();
            assert(e["state"].HasMember("bri"));
            assert(e["state"]["bri"].IsInt());
            if ( e["state"]["bri"].GetInt() < 0 )
            {
                d["lights"][c]["state"]["bri"].SetInt(0);
            }
            else if ( e["state"]["bri"].GetInt() > 255 )
            {
                d["lights"][c]["state"]["bri"].SetInt(255);
            }
            lights[i].bri = (int)((double)e["state"]["bri"].GetInt() / 255 * 100);
            writer.StartObject();
            writer.Key("name");
            writer.String(lights[i].name.c_str());
            writer.Key("id");
            writer.String(lights[i].id.c_str());
            writer.Key("on");
            writer.Bool(lights[i].on);
            writer.Key("brightness");
            writer.Int(lights[i].bri);
            writer.EndObject();
            i++;
        }
        writer.EndArray();
        cout << sb.GetString() << endl;
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    while(true)
    {
        sleep(100);
        lights = update_lights(lights);
    }
    return 0;
}

vector<HueLight> update_lights(vector<HueLight> lights)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init(); //curl object
    CURLcode res;
    Document d;
    d.SetObject();
    assert(d.IsObject());
    string s1;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:80/api/newdeveloper");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //only for https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); //only for https
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlwrite_func_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s1);
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }
    if (s1 != "")
    {
        const char* json = s1.c_str();
        d.Parse(json);
    }
    const Value& b = d["lights"];
    for (Value::MemberIterator itr_outter = d["groups"].MemberBegin(); itr_outter !=d["groups"].MemberEnd(); ++itr_outter)
    {
        const Value& a = d["groups"][itr_outter->name.GetString()]["lights"];
        int i = 0;
        StringBuffer sb;
        PrettyWriter<StringBuffer> writersb(sb);
        StringBuffer sd;
        PrettyWriter<StringBuffer> writersd(sd);
        for (Value::ConstMemberIterator itr = b.MemberBegin(); itr != b.MemberEnd(); ++itr)
        {
            const Value& c = a[i];
            const Value& e = b[c];
            assert(e.HasMember("state"));
            assert(e["state"].HasMember("on"));
            assert(e["state"]["on"].IsBool());
            assert(e["state"].HasMember("bri"));
            assert(e["state"]["bri"].IsInt());
            if ( e["state"]["bri"].GetInt() < 0 )
            {
                d["lights"][c]["state"]["bri"].SetInt(0);
            }
            else if ( e["state"]["bri"].GetInt() > 255 )
            {
                d["lights"][c]["state"]["bri"].SetInt(255);
            }
            if (lights[i].on != e["state"]["on"].GetBool()) {
                lights[i].on = e["state"]["on"].GetBool();
                lights[i].id = c.GetString();
                writersb.StartObject();
                writersb.Key("id");
                writersb.String(lights[i].id.c_str());
                writersb.Key("on");
                writersb.Bool(lights[i].on);
                writersb.EndObject();
                cout << sb.GetString() << endl;
            }
            if (lights[i].bri != (int)((double)e["state"]["bri"].GetInt() / 255 * 100)) {
                lights[i].bri = (int)((double)e["state"]["bri"].GetInt() / 255 * 100);
                lights[i].id = c.GetString();
                writersd.StartObject();
                writersd.Key("id");
                writersd.String(lights[i].id.c_str());
                writersd.Key("brightness");
                writersd.Int(lights[i].bri);
                writersd.EndObject();
                cout << sd.GetString() << endl;
            }
            i++;
        }
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return(lights);
}

size_t curlwrite_func_string(void *contents, size_t size, size_t nmemb, string *s)
{
    size_t newLength = size*nmemb;
    size_t oldLength = s->size();
    try
    {
        s->resize(oldLength + newLength);
    }
    catch(std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }
    copy((char*)contents,(char*)contents+newLength,s->begin()+oldLength);
    return size*nmemb;
}

void sleep(int time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}
