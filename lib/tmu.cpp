#include <cstdlib>
#include <cstring>

#include <GL/gl.h>

#include "macs.hpp"
#include "macs-internals.hpp"

using namespace macs;
using namespace macs::internals;


tmu::tmu(int u):
    unit(u),
    assigned(NULL)
{
}

void tmu::operator=(root *tex)
{
    if (tex == assigned)
        return;


    glActiveTexture(GL_TEXTURE0 + unit);


    if ((assigned != NULL) && ((tex == NULL) || (tex->type != assigned->type)))
    {
        if (assigned->type == root::texture)
        {
            dbgprintf("[tmu%i] Disabling 2D textures.\n", unit);
            glDisable(GL_TEXTURE_2D);
        }
        else
        {
            dbgprintf("[tmu%i] Disabling 3D textures.\n", unit);
            glDisable(GL_TEXTURE_3D);
        }
    }


    if (tex == NULL)
        dbgprintf("[tmu%i] Detaching texture.\n", unit);
    else
    {
        switch (tex->type)
        {
            case root::texture:
                if ((assigned == NULL) || (assigned->type != root::texture))
                {
                    glEnable(GL_TEXTURE_2D);
                    dbgprintf("[tmu%i] Enabled 2D textures.\n", unit);
                }

                dbgprintf("[tmu%i] Attaching texture “%s”.\n", unit, ((texture *)tex)->name);

                glBindTexture(GL_TEXTURE_2D, ((texture *)tex)->id);
                break;

            case root::texture_array:
                if ((assigned == NULL) || (assigned->type != root::texture_array))
                {
                    glEnable(GL_TEXTURE_3D);
                    dbgprintf("[tmu%i] Enabled 3D textures.\n", unit);
                }

                dbgprintf("[tmu%i] Attaching texture array “%s”.\n", unit, ((texture_array *)tex)->name);

                glBindTexture(GL_TEXTURE_3D, ((texture_array *)tex)->id);
                break;

            default:
                throw exc::inv_type;
        }
    }


    assigned = tex;
}



tmu_manager::tmu_manager(int u):
    units(u)
{
    tmus = new tmu[u];
    definitely = new bool[u];

    for (int i = 0; i < u; i++)
        tmus[i].reassign(i);
}

tmu_manager::~tmu_manager(void)
{
    delete tmus;
    delete definitely;
}

void tmu_manager::loosen(void)
{
    memset(definitely, 0, sizeof(bool) * units);
}

bool tmu_manager::operator&=(root *tex)
{
    for (int i = 0; i < units; i++)
    {
        if (tmus[i] == tex)
        {
            definitely[i] = true;
            return true;
        }
    }

    return false;
}

void tmu_manager::operator+=(root *tex)
{
    for (int i = 0; i < units; i++)
    {
        if (!definitely[i])
        {
            definitely[i] = true;
            tmus[i] = tex;
            return;
        }
    }

    throw exc::rsrc_lim_exc;
}

void tmu_manager::update(void)
{
    for (int i = 0; i < units; i++)
        if (!definitely[i] && (tmus[i] != NULL))
            tmus[i] = NULL;
}
