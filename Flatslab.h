#ifndef FLATSLAB_H
#define FLATSLAB_H

#include "Shape.h"
#include <vector>
#include <cmath>
#include <algorithm>

using Pts = std::vector<std::pair<float,float>>;

class FlatSlab : public Shape
{
public:
    FlatSlab(const Pts& pts, float y, float r, float g, float b)
        : Shape(r,g,b), m_pts(pts), m_y(y) {}

    void build() override
    {
        if(m_pts.size() < 3) return;
        std::vector<float> filled, wire;

        std::vector<int> idx;
        for(int i=0;i<(int)m_pts.size();++i) idx.push_back(i);

        // Ensure counter-clockwise winding
        float area = 0.f;
        int n = (int)idx.size();
        for(int i=0;i<n;++i){
            int j=(i+1)%n;
            area += m_pts[i].first * m_pts[j].second;
            area -= m_pts[j].first * m_pts[i].second;
        }
        if(area < 0) std::reverse(idx.begin(), idx.end());

        // Ear-clip triangulation
        int safety = 0;
        while(idx.size() > 3 && safety++ < 10000)
        {
            bool clipped = false;
            int sz = (int)idx.size();
            for(int i=0;i<sz;++i)
            {
                int a=idx[(i-1+sz)%sz];
                int b=idx[i];
                int c=idx[(i+1)%sz];
                if(!isEar(a,b,c,idx)) continue;
                // Force upward normal (0,1,0) on every vertex
                pushVertex(filled, m_pts[a].first,m_y,m_pts[a].second, 0.f,1.f,0.f);
                pushVertex(filled, m_pts[b].first,m_y,m_pts[b].second, 0.f,1.f,0.f);
                pushVertex(filled, m_pts[c].first,m_y,m_pts[c].second, 0.f,1.f,0.f);
                idx.erase(idx.begin()+i);
                clipped = true;
                break;
            }
            if(!clipped) break;
        }
        // Last triangle
        if(idx.size() == 3){
            pushVertex(filled, m_pts[idx[0]].first,m_y,m_pts[idx[0]].second, 0.f,1.f,0.f);
            pushVertex(filled, m_pts[idx[1]].first,m_y,m_pts[idx[1]].second, 0.f,1.f,0.f);
            pushVertex(filled, m_pts[idx[2]].first,m_y,m_pts[idx[2]].second, 0.f,1.f,0.f);
        }

        // Wireframe outline
        int orig = (int)m_pts.size();
        for(int i=0;i<orig;++i){
            int j=(i+1)%orig;
            pushLine(wire,
                m_pts[i].first, m_y, m_pts[i].second,
                m_pts[j].first, m_y, m_pts[j].second);
        }

        buildBuffers(filled, wire);
    }

private:
    Pts   m_pts;
    float m_y;

    float cross2D(int a, int b, int c) const {
        float ax=m_pts[b].first -m_pts[a].first;
        float az=m_pts[b].second-m_pts[a].second;
        float bx=m_pts[c].first -m_pts[a].first;
        float bz=m_pts[c].second-m_pts[a].second;
        return ax*bz - az*bx;
    }

    bool pointInTriangle(int p, int a, int b, int c) const {
        if(p==a||p==b||p==c) return false;
        float d1=cross2D(a,b,p);
        float d2=cross2D(b,c,p);
        float d3=cross2D(c,a,p);
        bool has_neg=(d1<0)||(d2<0)||(d3<0);
        bool has_pos=(d1>0)||(d2>0)||(d3>0);
        return !(has_neg && has_pos);
    }

    bool isEar(int a, int b, int c, const std::vector<int>& idx) const {
        if(cross2D(a,b,c) <= 0) return false;
        for(int v : idx){
            if(v==a||v==b||v==c) continue;
            if(pointInTriangle(v,a,b,c)) return false;
        }
        return true;
    }
};

#endif // FLATSLAB_H