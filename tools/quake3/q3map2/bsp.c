/* -------------------------------------------------------------------------------

   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */



/* marker */
#define BSP_C



/* dependencies */
#include "q3map2.h"



/* -------------------------------------------------------------------------------

   functions

   ------------------------------------------------------------------------------- */

/*
   ProcessAdvertisements()
   copies advertisement info into the BSP structures
 */

static void ProcessAdvertisements( void ) {
	int i;
	const char*         className;
	const char*         modelKey;
	int modelNum;
	bspModel_t*         adModel;
	bspDrawSurface_t*   adSurface;

	Sys_FPrintf( SYS_VRB, "--- ProcessAdvertisements ---\n" );

	for ( i = 0; i < numEntities; i++ ) {

		/* is an advertisement? */
		className = ValueForKey( &entities[ i ], "classname" );

		if ( !Q_stricmp( "advertisement", className ) ) {

			modelKey = ValueForKey( &entities[ i ], "model" );

			if ( strlen( modelKey ) > MAX_QPATH - 1 ) {
				Error( "Model Key for entity exceeds ad struct string length." );
			}
			else {
				if ( numBSPAds < MAX_MAP_ADVERTISEMENTS ) {
					bspAds[numBSPAds].cellId = IntForKey( &entities[ i ], "cellId" );
					strncpy( bspAds[numBSPAds].model, modelKey, sizeof( bspAds[numBSPAds].model ) );

					modelKey++;
					modelNum = atoi( modelKey );
					adModel = &bspModels[modelNum];

					if ( adModel->numBSPSurfaces != 1 ) {
						Error( "Ad cell id %d has more than one surface.", bspAds[numBSPAds].cellId );
					}

					adSurface = &bspDrawSurfaces[adModel->firstBSPSurface];

					// store the normal for use at run time.. all ad verts are assumed to
					// have identical normals (because they should be a simple rectangle)
					// so just use the first vert's normal
					VectorCopy( bspDrawVerts[adSurface->firstVert].normal, bspAds[numBSPAds].normal );

					// store the ad quad for quick use at run time
					if ( adSurface->surfaceType == MST_PATCH ) {
						int v0 = adSurface->firstVert + adSurface->patchHeight - 1;
						int v1 = adSurface->firstVert + adSurface->numVerts - 1;
						int v2 = adSurface->firstVert + adSurface->numVerts - adSurface->patchWidth;
						int v3 = adSurface->firstVert;
						VectorCopy( bspDrawVerts[v0].xyz, bspAds[numBSPAds].rect[0] );
						VectorCopy( bspDrawVerts[v1].xyz, bspAds[numBSPAds].rect[1] );
						VectorCopy( bspDrawVerts[v2].xyz, bspAds[numBSPAds].rect[2] );
						VectorCopy( bspDrawVerts[v3].xyz, bspAds[numBSPAds].rect[3] );
					}
					else {
						Error( "Ad cell %d has an unsupported Ad Surface type.", bspAds[numBSPAds].cellId );
					}

					numBSPAds++;
				}
				else {
					Error( "Maximum number of map advertisements exceeded." );
				}
			}
		}
	}

	Sys_FPrintf( SYS_VRB, "%9d in-game advertisements\n", numBSPAds );
}

/*
   SetCloneModelNumbers() - ydnar
   sets the model numbers for brush entities
 */

static void SetCloneModelNumbers( void ){
	int i, j;
	int models;
	char modelValue[ 10 ];
	const char  *value, *value2, *value3;


	/* start with 1 (worldspawn is model 0) */
	models = 1;
	for ( i = 1; i < numEntities; i++ )
	{
		/* only entities with brushes or patches get a model number */
		if ( entities[ i ].brushes == NULL && entities[ i ].patches == NULL ) {
			continue;
		}

		/* is this a clone? */
		value = ValueForKey( &entities[ i ], "_ins" );
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( &entities[ i ], "_instance" );
		}
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( &entities[ i ], "_clone" );
		}
		if ( value[ 0 ] != '\0' ) {
			continue;
		}

		/* add the model key */
		sprintf( modelValue, "*%d", models );
		SetKeyValue( &entities[ i ], "model", modelValue );

		/* increment model count */
		models++;
	}

	/* fix up clones */
	for ( i = 1; i < numEntities; i++ )
	{
		/* only entities with brushes or patches get a model number */
		if ( entities[ i ].brushes == NULL && entities[ i ].patches == NULL ) {
			continue;
		}

		/* is this a clone? */
		value = ValueForKey( &entities[ i ], "_ins" );
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( &entities[ i ], "_instance" );
		}
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( &entities[ i ], "_clone" );
		}
		if ( value[ 0 ] == '\0' ) {
			continue;
		}

		/* find an entity with matching clone name */
		for ( j = 0; j < numEntities; j++ )
		{
			/* is this a clone parent? */
			value2 = ValueForKey( &entities[ j ], "_clonename" );
			if ( value2[ 0 ] == '\0' ) {
				continue;
			}

			/* do they match? */
			if ( strcmp( value, value2 ) == 0 ) {
				/* get the model num */
				value3 = ValueForKey( &entities[ j ], "model" );
				if ( value3[ 0 ] == '\0' ) {
					Sys_Printf( "WARNING: Cloned entity %s referenced entity without model\n", value2 );
					continue;
				}
				models = atoi( &value2[ 1 ] );

				/* add the model key */
				sprintf( modelValue, "*%d", models );
				SetKeyValue( &entities[ i ], "model", modelValue );

				/* nuke the brushes/patches for this entity (fixme: leak!) */
				entities[ i ].brushes = NULL;
				entities[ i ].patches = NULL;
			}
		}
	}
}



/*
   FixBrushSides() - ydnar
   matches brushsides back to their appropriate drawsurface and shader
 */

static void FixBrushSides( entity_t *e ){
	int i;
	mapDrawSurface_t    *ds;
	sideRef_t           *sideRef;
	bspBrushSide_t      *side;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- FixBrushSides ---\n" );

	/* walk list of drawsurfaces */
	for ( i = e->firstDrawSurf; i < numMapDrawSurfs; i++ )
	{
		/* get surface and try to early out */
		ds = &mapDrawSurfs[ i ];
		if ( ds->outputNum < 0 ) {
			continue;
		}

		/* walk sideref list */
		for ( sideRef = ds->sideRef; sideRef != NULL; sideRef = sideRef->next )
		{
			/* get bsp brush side */
			if ( sideRef->side == NULL || sideRef->side->outputNum < 0 ) {
				continue;
			}
			side = &bspBrushSides[ sideRef->side->outputNum ];

			/* set drawsurface */
			side->surfaceNum = ds->outputNum;
			//%	Sys_FPrintf( SYS_VRB, "DS: %7d Side: %7d     ", ds->outputNum, sideRef->side->outputNum );

			/* set shader */
			if ( strcmp( bspShaders[ side->shaderNum ].shader, ds->shaderInfo->shader ) ) {
				//%	Sys_FPrintf( SYS_VRB, "Remapping %s to %s\n", bspShaders[ side->shaderNum ].shader, ds->shaderInfo->shader );
				side->shaderNum = EmitShader( ds->shaderInfo->shader, &ds->shaderInfo->contentFlags, &ds->shaderInfo->surfaceFlags );
			}
		}
	}
}



/*
   ProcessWorldModel()
   creates a full bsp + surfaces for the worldspawn entity
 */

void ProcessWorldModel( const char *portalFilePath, const char *lineFilePath ){
	int i, s;
	entity_t    *e;
	tree_t      *tree;
	face_t      *faces;
	qboolean ignoreLeaks, leaked;
	char level[ 2 ], shader[ 1024 ];
	const char  *value;
	int leakStatus;

	/* sets integer blockSize from worldspawn "_blocksize" key if it exists */
	value = ValueForKey( &entities[ 0 ], "_blocksize" );
	if ( value[ 0 ] == '\0' ) {
		value = ValueForKey( &entities[ 0 ], "blocksize" );
	}
	if ( value[ 0 ] == '\0' ) {
		value = ValueForKey( &entities[ 0 ], "chopsize" );  /* sof2 */
	}
	if ( value[ 0 ] != '\0' ) {
		/* scan 3 numbers */
		s = sscanf( value, "%d %d %d", &blockSize[ 0 ], &blockSize[ 1 ], &blockSize[ 2 ] );

		/* handle legacy case */
		if ( s == 1 ) {
			blockSize[ 1 ] = blockSize[ 0 ];
			blockSize[ 2 ] = blockSize[ 0 ];
		}
	}
	Sys_Printf( "block size = { %d %d %d }\n", blockSize[ 0 ], blockSize[ 1 ], blockSize[ 2 ] );

	/* sof2: ignore leaks? */
	value = ValueForKey( &entities[ 0 ], "_ignoreleaks" );  /* ydnar */
	if ( value[ 0 ] == '\0' ) {
		value = ValueForKey( &entities[ 0 ], "ignoreleaks" );
	}
	if ( value[ 0 ] == '1' ) {
		ignoreLeaks = qtrue;
	}
	else{
		ignoreLeaks = qfalse;
	}

	/* begin worldspawn model */
	BeginModel();
	e = &entities[ 0 ];
	e->firstDrawSurf = 0;

	/* ydnar: gs mods */
	ClearMetaTriangles();

	/* check for patches with adjacent edges that need to lod together */
	PatchMapDrawSurfs( e );

	/* build an initial bsp tree using all of the sides of all of the structural brushes */
	faces = MakeStructuralBSPFaceList( entities[ 0 ].brushes );
	tree = FaceBSP( faces );
	MakeTreePortals( tree );
	FilterStructuralBrushesIntoTree( e, tree );

	/* see if the bsp is completely enclosed */
	leakStatus = FloodEntities( tree );
	if ( ignoreLeaks ) {
		if ( leakStatus == FLOODENTITIES_LEAKED ) {
			leakStatus = FLOODENTITIES_GOOD;
		}
	}

	if ( leakStatus == FLOODENTITIES_GOOD ) {
		leaked = qfalse;
	}
	else
	{
		leaked = qtrue;

		Sys_FPrintf( SYS_NOXML, "**********************\n" );
		Sys_FPrintf( SYS_NOXML, "******* leaked *******\n" );
		Sys_FPrintf( SYS_NOXML, "**********************\n" );
	
		level[0] = (int) '0' + SYS_ERR;
		level[1] = 0;
	
		if ( leaktest ) {
			Sys_Printf( "--- MAP LEAKED, ABORTING LEAKTEST ---\n" );
			exit( 0 );
		}
	}

	if ( leakStatus != FLOODENTITIES_EMPTY ) { /* if no entities exist, this would accidentally the whole map, and that IS bad */
		/* rebuild a better bsp tree using only the sides that are visible from the inside */
		FillOutside( tree->headnode );

		/* chop the sides to the convex hull of their visible fragments, giving us the smallest polygons */
		ClipSidesIntoTree( e, tree );

		/* build a visible face tree (same thing as the initial bsp tree but after reducing the faces) */
		faces = MakeVisibleBSPFaceList( entities[ 0 ].brushes );
		FreeTree( tree );
		tree = FaceBSP( faces );
		MakeTreePortals( tree );
		FilterStructuralBrushesIntoTree( e, tree );

		/* ydnar: flood again for skybox */
		if ( skyboxPresent ) {
			FloodEntities( tree );
		}
	}

	/* save out information for visibility processing */
	NumberClusters( tree );
	if ( !leaked ) {
		WritePortalFile( tree, portalFilePath );
	}

	/* flood from entities */
	FloodAreas( tree );

	/* create drawsurfs for triangle models */
	AddTriangleModels( e );

	/* create drawsurfs for surface models */
	AddEntitySurfaceModels( e );

	/* generate bsp brushes from map brushes */
	EmitBrushes( e->brushes, &e->firstBrush, &e->numBrushes );

	/* add references to the detail brushes */
	FilterDetailBrushesIntoTree( e, tree );

	/* drawsurfs that cross fog boundaries will need to be split along the fog boundary */
	if ( !nofog ) {
		FogDrawSurfaces( e );
	}

	/* subdivide each drawsurf as required by shader tesselation */
	if ( !nosubdivide ) {
		SubdivideFaceSurfaces( e, tree );
	}

	/* add in any vertexes required to fix t-junctions */
	if ( !notjunc ) {
		FixTJunctions( e );
	}

	/* ydnar: classify the surfaces */
	ClassifyEntitySurfaces( e );

	/* ydnar: project decals */
	MakeEntityDecals( e );

	/* ydnar: meta surfaces */
	MakeEntityMetaTriangles( e );
	SmoothMetaTriangles();
	FixMetaTJunctions();
	MergeMetaTriangles();

	/* ydnar: debug portals */
	if ( debugPortals ) {
		MakeDebugPortalSurfs( tree );
	}

	/* ydnar: fog hull */
	value = ValueForKey( &entities[ 0 ], "_foghull" );
	if ( value[ 0 ] != '\0' ) {
		sprintf( shader, "textures/%s", value );
		MakeFogHullSurfs( e, tree, shader );
	}

	/* ydnar: bug 645: do flares for lights */
	for ( i = 0; i < numEntities && emitFlares; i++ )
	{
		entity_t    *light, *target;
		const char  *value, *flareShader;
		vec3_t origin, targetOrigin, normal, color;
		int lightStyle;


		/* get light */
		light = &entities[ i ];
		value = ValueForKey( light, "classname" );
		if ( !strcmp( value, "light" ) ) {
			/* get flare shader */
			flareShader = ValueForKey( light, "_flareshader" );
			value = ValueForKey( light, "_flare" );
			if ( flareShader[ 0 ] != '\0' || value[ 0 ] != '\0' ) {
				/* get specifics */
				GetVectorForKey( light, "origin", origin );
				GetVectorForKey( light, "_color", color );
				lightStyle = IntForKey( light, "_style" );
				if ( lightStyle == 0 ) {
					lightStyle = IntForKey( light, "style" );
				}

				/* handle directional spotlights */
				value = ValueForKey( light, "target" );
				if ( value[ 0 ] != '\0' ) {
					/* get target light */
					target = FindTargetEntity( value );
					if ( target != NULL ) {
						GetVectorForKey( target, "origin", targetOrigin );
						VectorSubtract( targetOrigin, origin, normal );
						VectorNormalize( normal, normal );
					}
				}
				else{
					//%	VectorClear( normal );
					VectorSet( normal, 0, 0, -1 );
				}

				if ( colorsRGB ) {
					color[0] = Image_LinearFloatFromsRGBFloat( color[0] );
					color[1] = Image_LinearFloatFromsRGBFloat( color[1] );
					color[2] = Image_LinearFloatFromsRGBFloat( color[2] );
				}

				/* create the flare surface (note shader defaults automatically) */
				DrawSurfaceForFlare( mapEntityNum, origin, normal, color, flareShader, lightStyle );
			}
		}
	}

	/* add references to the final drawsurfs in the apropriate clusters */
	FilterDrawsurfsIntoTree( e, tree );

	/* match drawsurfaces back to original brushsides (sof2) */
	FixBrushSides( e );

	/* finish */
	EndModel( e, tree->headnode );
	FreeTree( tree );
}



/*
   ProcessSubModel()
   creates bsp + surfaces for other brush models
 */

void ProcessSubModel( void ){
	entity_t    *e;
	tree_t      *tree;
	brush_t     *b, *bc;
	node_t      *node;


	/* start a brush model */
	BeginModel();
	e = &entities[ mapEntityNum ];
	e->firstDrawSurf = numMapDrawSurfs;

	/* ydnar: gs mods */
	ClearMetaTriangles();

	/* check for patches with adjacent edges that need to lod together */
	PatchMapDrawSurfs( e );

	/* allocate a tree */
	node = AllocNode();
	node->planenum = PLANENUM_LEAF;
	tree = AllocTree();
	tree->headnode = node;

	/* add the sides to the tree */
	ClipSidesIntoTree( e, tree );

	/* ydnar: create drawsurfs for triangle models */
	AddTriangleModels( e );

	/* create drawsurfs for surface models */
	AddEntitySurfaceModels( e );

	/* generate bsp brushes from map brushes */
	EmitBrushes( e->brushes, &e->firstBrush, &e->numBrushes );

	/* just put all the brushes in headnode */
	for ( b = e->brushes; b; b = b->next )
	{
		bc = CopyBrush( b );
		bc->next = node->brushlist;
		node->brushlist = bc;
	}

	/* subdivide each drawsurf as required by shader tesselation */
	if ( !nosubdivide ) {
		SubdivideFaceSurfaces( e, tree );
	}

	/* add in any vertexes required to fix t-junctions */
	if ( !notjunc ) {
		FixTJunctions( e );
	}

	/* ydnar: classify the surfaces and project lightmaps */
	ClassifyEntitySurfaces( e );

	/* ydnar: project decals */
	MakeEntityDecals( e );

	/* ydnar: meta surfaces */
	MakeEntityMetaTriangles( e );
	SmoothMetaTriangles();
	FixMetaTJunctions();
	MergeMetaTriangles();

	/* add references to the final drawsurfs in the apropriate clusters */
	FilterDrawsurfsIntoTree( e, tree );

	/* match drawsurfaces back to original brushsides (sof2) */
	FixBrushSides( e );

	/* finish */
	EndModel( e, node );
	FreeTree( tree );
}



/*
   ProcessModels()
   process world + other models into the bsp
 */

void ProcessModels( const char *portalFilePath, const char *lineFilePath ){
	qboolean oldVerbose;
	entity_t    *entity;


	/* preserve -v setting */
	oldVerbose = verbose;

	/* start a new bsp */
	BeginBSPFile();

	/* create map fogs */
	CreateMapFogs();

	/* walk entity list */
	for ( mapEntityNum = 0; mapEntityNum < numEntities; mapEntityNum++ )
	{
		/* get entity */
		entity = &entities[ mapEntityNum ];
		if ( entity->brushes == NULL && entity->patches == NULL ) {
			continue;
		}

		/* process the model */
		Sys_FPrintf( SYS_VRB, "############### model %i ###############\n", numBSPModels );
		if ( mapEntityNum == 0 ) {
			ProcessWorldModel(portalFilePath, lineFilePath);
		}
		else{
			ProcessSubModel();
		}

		/* potentially turn off the deluge of text */
		verbose = verboseEntities;
	}

	/* restore -v setting */
	verbose = oldVerbose;

	/* write fogs */
	EmitFogs();

	/* vortex: emit meta stats */
	EmitMetaStats();
}



/*
   OnlyEnts()
   this is probably broken unless teamed with a radiant version that preserves entity order
 */

void OnlyEnts( const char *BSPFilePath ){
	char save_cmdline[1024], save_version[1024], save_gridsize[1024];
	const char *p;

	/* note it */
	Sys_Printf( "--- OnlyEnts ---\n" );

	LoadBSPFile( BSPFilePath );

	ParseEntities();
	p = ValueForKey( &entities[0], "_q3map2_cmdline" );
	strncpy( save_cmdline, p, sizeof( save_cmdline ) );
	save_cmdline[sizeof( save_cmdline ) - 1] = 0;
	p = ValueForKey( &entities[0], "_q3map2_version" );
	strncpy( save_version, p, sizeof( save_version ) );
	save_version[sizeof( save_version ) - 1] = 0;
	p = ValueForKey( &entities[0], "gridsize" );
	strncpy( save_gridsize, p, sizeof( save_gridsize ) );
	save_gridsize[sizeof( save_gridsize ) - 1] = 0;

	numEntities = 0;

	LoadShaderInfo();
	LoadMapFile( name, qfalse, qfalse );
	SetModelNumbers();
	SetLightStyles();

	if ( *save_cmdline ) {
		SetKeyValue( &entities[0], "_q3map2_cmdline", save_cmdline );
	}
	if ( *save_version ) {
		SetKeyValue( &entities[0], "_q3map2_version", save_version );
	}
	if ( *save_gridsize ) {
		SetKeyValue( &entities[0], "gridsize", save_gridsize );
	}

	numBSPEntities = numEntities;
	UnparseEntities();

	WriteBSPFile( BSPFilePath );
}

void ResetGlobals( void )
{
	// A bit of a hack! Reset all global vars, since we can run multiple times now rather than being a 
	// fire-and-forget run of an executable.

	// This is nasty, but necessary.
	ResetBspFile();
	ResetAllSurfaceMetaData();
	ResetMap();
	ResetPortals();
	ResetTJunctions();
	ResetSurfaceExtra();

// /* general */
 	numImages =( 0 );

 	numPicoModels =( 0 );
 	numShaderInfo =( 0 );
 	numVertexRemaps =( 0 );
 	numCustSurfaceParms =( 0 );

// /* bsp */
	numMapEntities = ( 0 );
	mapEntityNum = ( 0 );
	entitySourceBrushes =  0;

	free(mapplanes);
	mapplanes = NULL;
	nummapplanes = ( 0 );                    /* nummapplanes will always be even */
	allocatedmapplanes = ( 0 );
	numMapPatches = 0;
	
	mapMins[0] = mapMins[1] = mapMins[2] = (0);
	mapMaxs[0] = mapMaxs[1] = mapMaxs[2] = (0);
	
	defaultFogNum = ( -1 );                  /* ydnar: cleaner fog handling */
	numMapFogs = ( 0 );

 	mapEnt = NULL;

	free(buildBrush);
 	buildBrush = NULL;
	
 	numActiveBrushes = 0;
 	g_bBrushPrimit = 0;

 	numStrippedLights = ( 0 );

// /* surface stuff */
	free(mapDrawSurfs);
	mapDrawSurfs = ( NULL );
	numMapDrawSurfs = 0;

	for (int i = 0; i < NUM_SURFACE_TYPES; i++)
	{
		numSurfacesByType[i] = 0;
	}
	numClearedSurfaces = 0;
	numStripSurfaces = 0;
	numMaxAreaSurfaces = 0;
	numFanSurfaces = 0;
	numMergedSurfaces = 0;
	numMergedVerts = 0;

	numRedundantIndexes = 0;

	numSurfaceModels = ( 0 );

 	skyboxPresent = ( qfalse );
 	skyboxArea = ( -1 );
	
	numEntities = ( 0 );
	numBSPEntities = ( 0 );
	allocatedEntities = ( 0 );

	free(entities);
	entities = ( NULL );

	numBSPModels = ( 0 );
	allocatedBSPModels = ( 0 );
	free(bspModels);
	bspModels = ( NULL );

	numBSPShaders = ( 0 );
	allocatedBSPShaders = ( 0 );
	free(bspShaders);
	bspShaders = ( NULL );

	bspEntDataSize = ( 0 );
	allocatedBSPEntData = ( 0 );
	free(bspEntData);
	bspEntData = NULL;

	numBSPLeafs = ( 0 );
	numBSPPlanes = ( 0 );
	
	numBSPNodes = ( 0 );
	allocatedBSPNodes = ( 0 );
	free(bspNodes);
	bspNodes = ( NULL );

	numBSPLeafSurfaces = ( 0 );
	numBSPLeafBrushes = ( 0 );
	numBSPBrushes = ( 0 );
	numBSPBrushSides = ( 0 );

	numBSPLightBytes = ( 0 );
	free(bspLightBytes);
	bspLightBytes = ( NULL );

	numBSPGridPoints = ( 0 );
	free(bspGridPoints);
	bspGridPoints = ( NULL );

	numBSPVisBytes = ( 0 );
	
	numBSPDrawVerts = ( 0 );
	free(bspDrawVerts);
	bspDrawVerts = ( NULL );

	numBSPDrawIndexes = ( 0 );
	allocatedBSPDrawIndexes = ( 0 );
	free(bspDrawIndexes);
	bspDrawIndexes = ( NULL );

	numBSPDrawSurfaces = ( 0 );
	numBSPFogs = ( 0 );	
	numBSPAds = ( 0 );
}

/*
   BSPMain() - ydnar
   handles creation of a bsp from a map file
 */

int BSPMain( int argc, char **argv ){
	int i;
	char path[ 1024 ], tempSource[ 1024 ];
	qboolean onlyents = qfalse;
	char BSPFilePath [ 1024 ];
	char lineFilePath [ 1024 ];
	char portalFilePath [ 1024 ];
	char surfaceFilePath [ 1024 ];
	BSPFilePath[0] = 0;
	lineFilePath[0] = 0;
	portalFilePath[0] = 0;
	surfaceFilePath[0] = 0;


	/* note it */
	Sys_Printf( "--- BSP ---\n" );

	SetDrawSurfacesBuffer();
	mapDrawSurfs = safe_malloc( sizeof( mapDrawSurface_t ) * MAX_MAP_DRAW_SURFS );
	memset( mapDrawSurfs, 0, sizeof( mapDrawSurface_t ) * MAX_MAP_DRAW_SURFS );
	numMapDrawSurfs = 0;

	tempSource[ 0 ] = '\0';
	globalCelShader[0] = 0;

	/* set standard game flags */
	maxSurfaceVerts = game->maxSurfaceVerts;
	maxSurfaceIndexes = game->maxSurfaceIndexes;
	emitFlares = game->emitFlares;
	texturesRGB = game->texturesRGB;
	colorsRGB = game->colorsRGB;

	/* process arguments */
	for ( i = 1; i < ( argc - 1 ); i++ )
	{
		if ( !strcmp( argv[ i ], "-onlyents" ) ) {
			Sys_Printf( "Running entity-only compile\n" );
			onlyents = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-tempname" ) ) {
			strcpy( tempSource, argv[ ++i ] );
		}
		else if ( !strcmp( argv[ i ], "-tmpout" ) ) {
			strcpy( outbase, "/tmp" );
		}
		else if ( !strcmp( argv[ i ],  "-nowater" ) ) {
			Sys_Printf( "Disabling water\n" );
			nowater = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-keeplights" ) ) {
			keepLights = qtrue;
			Sys_Printf( "Leaving light entities on map after compile\n" );
		}
		else if ( !strcmp( argv[ i ],  "-nodetail" ) ) {
			Sys_Printf( "Ignoring detail brushes\n" ) ;
			nodetail = qtrue;
		}
		else if ( !strcmp( argv[ i ],  "-fulldetail" ) ) {
			Sys_Printf( "Turning detail brushes into structural brushes\n" );
			fulldetail = qtrue;
		}
		else if ( !strcmp( argv[ i ],  "-nofog" ) ) {
			Sys_Printf( "Fog volumes disabled\n" );
			nofog = qtrue;
		}
		else if ( !strcmp( argv[ i ],  "-nosubdivide" ) ) {
			Sys_Printf( "Disabling brush face subdivision\n" );
			nosubdivide = qtrue;
		}
		else if ( !strcmp( argv[ i ],  "-leaktest" ) ) {
			Sys_Printf( "Leaktest enabled\n" );
			leaktest = qtrue;
		}
		else if ( !strcmp( argv[ i ],  "-verboseentities" ) ) {
			Sys_Printf( "Verbose entities enabled\n" );
			verboseEntities = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-nocurves" ) ) {
			Sys_Printf( "Ignoring curved surfaces (patches)\n" );
			noCurveBrushes = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-notjunc" ) ) {
			Sys_Printf( "T-junction fixing disabled\n" );
			notjunc = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-fakemap" ) ) {
			Sys_Printf( "Generating fakemap.map\n" );
			fakemap = qtrue;
		}
		else if ( !strcmp( argv[ i ],  "-samplesize" ) ) {
			sampleSize = atoi( argv[ i + 1 ] );
			if ( sampleSize < 1 ) {
				sampleSize = 1;
			}
			i++;
			Sys_Printf( "Lightmap sample size set to %dx%d units\n", sampleSize, sampleSize );
		}
		else if ( !strcmp( argv[ i ], "-minsamplesize" ) ) {
			minSampleSize = atoi( argv[ i + 1 ] );
			if ( minSampleSize < 1 ) {
				minSampleSize = 1;
			}
			i++;
			Sys_Printf( "Minimum lightmap sample size set to %dx%d units\n", minSampleSize, minSampleSize );
		}
		else if ( !strcmp( argv[ i ],  "-custinfoparms" ) ) {
			Sys_Printf( "Custom info parms enabled\n" );
			useCustomInfoParms = qtrue;
		}

		/* sof2 args */
		else if ( !strcmp( argv[ i ], "-rename" ) ) {
			Sys_Printf( "Appending _bsp suffix to misc_model shaders (SOF2)\n" );
			renameModelShaders = qtrue;
		}

		/* ydnar args */
		else if ( !strcmp( argv[ i ],  "-ne" ) ) {
			normalEpsilon = atof( argv[ i + 1 ] );
			i++;
			Sys_Printf( "Normal epsilon set to %f\n", normalEpsilon );
		}
		else if ( !strcmp( argv[ i ],  "-de" ) ) {
			distanceEpsilon = atof( argv[ i + 1 ] );
			i++;
			Sys_Printf( "Distance epsilon set to %f\n", distanceEpsilon );
		}
		else if ( !strcmp( argv[ i ],  "-mv" ) ) {
			maxLMSurfaceVerts = atoi( argv[ i + 1 ] );
			if ( maxLMSurfaceVerts < 3 ) {
				maxLMSurfaceVerts = 3;
			}
			if ( maxLMSurfaceVerts > maxSurfaceVerts ) {
				maxSurfaceVerts = maxLMSurfaceVerts;
			}
			i++;
			Sys_Printf( "Maximum lightmapped surface vertex count set to %d\n", maxLMSurfaceVerts );
		}
		else if ( !strcmp( argv[ i ],  "-mi" ) ) {
			maxSurfaceIndexes = atoi( argv[ i + 1 ] );
			if ( maxSurfaceIndexes < 3 ) {
				maxSurfaceIndexes = 3;
			}
			i++;
			Sys_Printf( "Maximum per-surface index count set to %d\n", maxSurfaceIndexes );
		}
		else if ( !strcmp( argv[ i ], "-np" ) ) {
			npDegrees = atof( argv[ i + 1 ] );
			if ( npDegrees < 0.0f ) {
				npDegrees = 0.0f;
			}
			else if ( npDegrees > 0.0f ) {
				Sys_Printf( "Forcing nonplanar surfaces with a breaking angle of %f degrees\n", npDegrees );
			}
			i++;
		}
		else if ( !strcmp( argv[ i ],  "-snap" ) ) {
			bevelSnap = atoi( argv[ i + 1 ] );
			if ( bevelSnap < 0 ) {
				bevelSnap = 0;
			}
			i++;
			if ( bevelSnap > 0 ) {
				Sys_Printf( "Snapping brush bevel planes to %d units\n", bevelSnap );
			}
		}
		else if ( !strcmp( argv[ i ],  "-texrange" ) ) {
			texRange = atoi( argv[ i + 1 ] );
			if ( texRange < 0 ) {
				texRange = 0;
			}
			i++;
			Sys_Printf( "Limiting per-surface texture range to %d texels\n", texRange );
		}
		else if ( !strcmp( argv[ i ], "-nohint" ) ) {
			Sys_Printf( "Hint brushes disabled\n" );
			noHint = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-flat" ) ) {
			Sys_Printf( "Flatshading enabled\n" );
			flat = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-celshader" ) ) {
			++i;
			if ( argv[i][0] ) {
				sprintf( globalCelShader, "textures/%s", argv[ i ] );
			}
			else{
				*globalCelShader = 0;
			}
			Sys_Printf( "Global cel shader set to \"%s\"\n", globalCelShader );
		}
		else if ( !strcmp( argv[ i ], "-meta" ) ) {
			Sys_Printf( "Creating meta surfaces from brush faces\n" );
			meta = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-metaadequatescore" ) ) {
			metaAdequateScore = atoi( argv[ i + 1 ] );
			if ( metaAdequateScore < 0 ) {
				metaAdequateScore = -1;
			}
			i++;
			if ( metaAdequateScore >= 0 ) {
				Sys_Printf( "Setting ADEQUATE meta score to %d (see surface_meta.c)\n", metaAdequateScore );
			}
		}
		else if ( !strcmp( argv[ i ], "-metagoodscore" ) ) {
			metaGoodScore = atoi( argv[ i + 1 ] );
			if ( metaGoodScore < 0 ) {
				metaGoodScore = -1;
			}
			i++;
			if ( metaGoodScore >= 0 ) {
				Sys_Printf( "Setting GOOD meta score to %d (see surface_meta.c)\n", metaGoodScore );
			}
		}
		else if ( !strcmp( argv[ i ], "-metamaxbboxdistance" ) ) {
			metaMaxBBoxDistance = atof( argv[ i + 1 ] );
			if ( metaMaxBBoxDistance < 0 ) {
				metaMaxBBoxDistance = -1;
			}
			i++;
			if ( metaMaxBBoxDistance >= 0 ) {
				Sys_Printf( "Setting meta maximum bounding box distance to %f\n", metaMaxBBoxDistance );
			}
		}
		else if ( !strcmp( argv[ i ], "-patchmeta" ) ) {
			Sys_Printf( "Creating meta surfaces from patches\n" );
			patchMeta = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-flares" ) ) {
			Sys_Printf( "Flare surfaces enabled\n" );
			emitFlares = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-noflares" ) ) {
			Sys_Printf( "Flare surfaces disabled\n" );
			emitFlares = qfalse;
		}
		else if ( !strcmp( argv[ i ], "-skyfix" ) ) {
			Sys_Printf( "GL_CLAMP sky fix/hack/workaround enabled\n" );
			skyFixHack = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-debugsurfaces" ) ) {
			Sys_Printf( "emitting debug surfaces\n" );
			debugSurfaces = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-debuginset" ) ) {
			Sys_Printf( "Debug surface triangle insetting enabled\n" );
			debugInset = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-debugportals" ) ) {
			Sys_Printf( "Debug portal surfaces enabled\n" );
			debugPortals = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-sRGBtex" ) ) {
			texturesRGB = qtrue;
			Sys_Printf( "Textures are in sRGB\n" );
		}
		else if ( !strcmp( argv[ i ], "-nosRGBtex" ) ) {
			texturesRGB = qfalse;
			Sys_Printf( "Textures are linear\n" );
		}
		else if ( !strcmp( argv[ i ], "-sRGBcolor" ) ) {
			colorsRGB = qtrue;
			Sys_Printf( "Colors are in sRGB\n" );
		}
		else if ( !strcmp( argv[ i ], "-nosRGBcolor" ) ) {
			colorsRGB = qfalse;
			Sys_Printf( "Colors are linear\n" );
		}
		else if ( !strcmp( argv[ i ], "-nosRGB" ) ) {
			texturesRGB = qfalse;
			Sys_Printf( "Textures are linear\n" );
			colorsRGB = qfalse;
			Sys_Printf( "Colors are linear\n" );
		}
		else if ( !strcmp( argv[ i ], "-altsplit" ) ) {
			Sys_Printf( "Alternate BSP splitting (by 27) enabled\n" );
			bspAlternateSplitWeights = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-deep" ) ) {
			Sys_Printf( "Deep BSP tree generation enabled\n" );
			deepBSP = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-maxarea" ) ) {
			Sys_Printf( "Max Area face surface generation enabled\n" );
			maxAreaFaceSurface = qtrue;
		}
		else if ( !strcmp( argv[ i ], "-bspfile" ) )
		{
			strcpy( BSPFilePath, argv[i + 1] );
			i++;
			Sys_Printf( "Use %s as bsp file\n", BSPFilePath );
		}
		else if ( !strcmp( argv[ i ], "-linfile" ) )
		{
			strcpy( lineFilePath, argv[i + 1] );
			i++;
			Sys_Printf( "Use %s as line file\n", lineFilePath );
		}
		else if ( !strcmp( argv[ i ], "-prtfile" ) )
		{
			strcpy( portalFilePath, argv[i + 1] );
			i++;
			Sys_Printf( "Use %s as portal file\n", portalFilePath );
		}
		else if ( !strcmp( argv[ i ], "-srffile" ) )
		{
			strcpy( surfaceFilePath, argv[i + 1] );
			i++;
			Sys_Printf( "Use %s as surface file\n", surfaceFilePath );
		}
		else if ( !strcmp( argv[ i ], "-bsp" ) ) {
			Sys_Printf( "-bsp argument unnecessary\n" );
		}
		else
		{
			Sys_Printf( "WARNING: Unknown option \"%s\"\n", argv[ i ] );
		}
	}

	/* fixme: print more useful usage here */
	if ( i != ( argc - 1 ) ) {
		Error( "usage: q3map [options] mapfile" );
		return 1;
	}

	/* copy source name */
	strcpy( source, ExpandArg( argv[ i ] ) );
	StripExtension( source );

	/* ydnar: set default sample size */
	SetDefaultSampleSize( sampleSize );

	if (!BSPFilePath[0]) {
		sprintf( BSPFilePath, "%s.bsp", source );
	}
	if (!lineFilePath[0]) {
		sprintf( lineFilePath, "%s.lin", source );
	}
	if (!portalFilePath[0]) {
		sprintf( portalFilePath, "%s.prt", source );
	}
	if (!surfaceFilePath[0]) {
		sprintf( surfaceFilePath, "%s.srf", source );
	}

	/* delete portal, line and surface files */
	remove( portalFilePath );
	remove( lineFilePath );
	//%	remove( surfaceFilePath )	/* ydnar */

	/* expand mapname */
	strcpy( name, ExpandArg( argv[ i ] ) );
	if ( strcmp( name + strlen( name ) - 4, ".reg" ) ) {
		/* if we are doing a full map, delete the last saved region map */
		sprintf( path, "%s.reg", source );
		remove( path );
		DefaultExtension( name, ".map" );   /* might be .reg */
	}

	/* if onlyents, just grab the entites and resave */
	if ( onlyents ) {
		OnlyEnts( BSPFilePath );
		return 0;
	}

	/* load shaders */
	LoadShaderInfo();

	/* load original file from temp spot in case it was renamed by the editor on the way in */
	if ( strlen( tempSource ) > 0 ) {
		LoadMapFile( tempSource, qfalse, qfalse );
	}
	else{
		LoadMapFile( name, qfalse, qfalse );
	}

	/* div0: inject command line parameters */
	InjectCommandLine( argv, 1, argc - 1 );

	/* ydnar: decal setup */
	ProcessDecals();

	/* ydnar: cloned brush model entities */
	SetCloneModelNumbers();

	/* process world and submodels */
	ProcessModels( portalFilePath, lineFilePath );

	/* set light styles from targetted light entities */
	SetLightStyles();

	/* process in game advertisements */
	ProcessAdvertisements();

	/* finish and write bsp */
	EndBSPFile( qtrue, BSPFilePath, surfaceFilePath );

	/* remove temp map source file if appropriate */
	if ( strlen( tempSource ) > 0 ) {
		remove( tempSource );
	}

	/* return to sender */
	return 0;
}
