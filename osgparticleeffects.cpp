/* OpenSceneGraph example, osgparticleeffects.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/


#include <osgViewer/Viewer>

#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/io_utils>

#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>

//#include <osgText/Text>
//
//#include <osgParticle/ExplosionEffect>
//#include <osgParticle/ExplosionDebrisEffect>
//#include <osgParticle/SmokeEffect>
//#include <osgParticle/SmokeTrailEffect>
//#include <osgParticle/FireEffect>
#include <numeric>
#include <assert.h>
#include "H:\codes\testTiff\myTiffUtil.h"

//////////////////////////////////////////////////////////////////////////////
// MAIN SCENE GRAPH BUILDING FUNCTION
//////////////////////////////////////////////////////////////////////////////
struct MyStruct
{
	char *tex_file;
	char *profile_file;
	MyBuf buf;
	double x_scale, y_scale, z_scale;
};

float getz(float x, float y, float scale)
{
	return scale*(x*x + y*y);
}

void build_world(osg::Group *root, MyStruct const &par)
{

    osg::Geode* terrainGeode = new osg::Geode;
    // create terrain
    {
        osg::StateSet* stateset = new osg::StateSet();
		osg::Image* image = osgDB::readImageFile(par.tex_file ? par.tex_file: "Images/img_wallpaper.bmp");
        if (image)
        {
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setImage(image);
            stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
        }

        terrainGeode->setStateSet( stateset );

		

		osg::HeightField* grid = new osg::HeightField;

		if (par.profile_file)
		{
			int maxnwh = 500;
			double samplestepx = 1.0*par.buf.w / maxnwh;
			double samplestepy = std::max(1.0,1.0*par.buf.h / maxnwh);

			int nc = 1.0*par.buf.w / samplestepx;
			int nr = 1.0*par.buf.h / samplestepy;
			float zscale = par.z_scale;

			grid->allocate(nc, nr);
			grid->setXInterval(par.x_scale*samplestepx);
			grid->setYInterval(par.y_scale*samplestepy);

			assert(par.buf.type == TYPE_32F);

			for (int r = 0; r<nr; ++r)
			{
				for (int c = 0; c<nc; ++c)
				{
					int x = c*samplestepx;
					int y = r*samplestepy;
					const void *p = getp(par.buf, x, y);
					float z = *(float*)p *zscale;
					grid->setHeight(c, r, z);
				}
			}
		}
		else
		{
			float scale = 1;
			int nc = 380;
			int nr = 390;
			float zscale = 0.25 * 380 / getz((nc / 2), (nr / 2), 1);

			grid->allocate(nc, nr);
			grid->setXInterval(scale);
			grid->setYInterval(scale);

			for (int r = 0; r<nr; ++r)
			{
				for (int c = 0; c<nc; ++c)
				{
					float z = getz((c - nc / 2), (r - nr / 2), zscale);
					grid->setHeight(c, r, z);
				}
			}
		}
        
        terrainGeode->addDrawable(new osg::ShapeDrawable(grid));
        
        root->addChild(terrainGeode);
    }    

}

int main(int c, char **v)
{
	system("pause");

	MyStruct par;
	memset(&par, 0, sizeof(par));

	if (c > 1)
		par.tex_file = v[1];
	if (c > 2)
		par.profile_file = v[2];
	if (c > 3)
		par.x_scale = atof(v[3]);
	if (c > 4)
		par.y_scale = atof(v[4]);
	if (c > 5)
		par.z_scale = atof(v[5]);

	if (par.profile_file)
	{
		readTiff(par.profile_file, &par.buf);
	}
    // construct the viewer.
    osgViewer::Viewer viewer;

    // register the pick handler
    //viewer.addEventHandler(new PickHandler());
    
    osg::Group *root = new osg::Group;
    build_world(root, par);

    osgUtil::Optimizer optimizer;
    optimizer.optimize(root);
   
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);
        
    viewer.run();
	freeBuf(&par.buf);
}
