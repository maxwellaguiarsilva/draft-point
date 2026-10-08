//	vertex-shader
#version 460 core

const vec2 vertices[ 4 ] = vec2[ 4 ](
	 vec2( -1.0, -1.0 )
	,vec2(  1.0, -1.0 )
	,vec2( -1.0,  1.0 )
	,vec2(  1.0,  1.0 )
);

void main( ) { gl_Position = vec4( vertices[ gl_VertexID ], 0.0, 1.0 ); }
//	vertex-shader

//	fragment-shader
#version 460 core

layout( location = 0 ) uniform int sphere_count;
layout( location = 1 ) uniform vec2 resolution;

layout( std430, binding = 1 ) readonly buffer sphere_buffer { float sphere_data[]; };
layout( std430, binding = 0 ) readonly buffer environment_buffer { float environment_data[]; };

out vec4 final_color;

#define vec3_from( data, index ) vec3( data[ ( index ) ], data[ ( index ) + 1 ], data[ ( index ) + 2 ] )
#define vec4_from( data, index ) vec4( data[ ( index ) ], data[ ( index ) + 1 ], data[ ( index ) + 2 ], data[ ( index ) + 3 ] )
#define square( value ) ( ( value ) * ( value ) )

struct camera
{
	vec3	position;
	vec3	forward;
	vec3	right;
	vec3	up;
};

struct environment
{
	camera	camera;
	vec4	background_color;
	float	focal;
	float	ambient;
	float	volume;
};

environment get_environment( )
{
	return environment(
		camera(
			vec3_from( environment_data, 0 ),
			vec3_from( environment_data, 3 ),
			vec3_from( environment_data, 6 ),
			vec3_from( environment_data, 9 )
		),
		vec4_from( environment_data, 12 ),
		environment_data[ 16 ],
		environment_data[ 17 ],
		environment_data[ 18 ]
	);
}

struct sphere
{
	vec3	position;
	float	radius;
	vec4	color;
};

sphere get_sphere( int index )
{
	int	base_index = index * 8;
	return sphere( vec3_from( sphere_data, base_index + 0 ), sphere_data[ base_index + 3 ], vec4_from( sphere_data, base_index + 4 ));
}

void main( )
{
	vec2	input_coord = ( gl_FragCoord.xy - 0.5 * resolution ) * ( 2.0 / min( resolution.x, resolution.y ) );

	const	environment	current_environment = get_environment( );

	vec3	direction = current_environment.camera.forward * current_environment.focal + current_environment.camera.right * input_coord.x + current_environment.camera.up * input_coord.y;

	float	best_distance = 1e20;
	vec3	color = current_environment.background_color.rgb;

	for( int index = 0; index < sphere_count; ++index )
	{
		sphere	object = get_sphere( index );
		vec3	hypotenuse = object.position - current_environment.camera.position;
		float	dot_hypotenuse_direction = dot( hypotenuse, direction );
		if( dot_hypotenuse_direction <= 0.0 ) continue;
		float	opposite_leg_squared = dot( hypotenuse, hypotenuse ) - square( dot_hypotenuse_direction ) / dot( direction, direction );
		float	ratio = opposite_leg_squared / square( object.radius );
		float	distance = length( hypotenuse );

		if( distance < best_distance && ratio <= 1.0 )
		{
			best_distance = distance;
			color = object.color.rgb * ( current_environment.ambient + current_environment.volume * sqrt( 1.0 - clamp( ratio, 0.0, 1.0 ) ) );
		}
	}

	final_color = vec4( color, 1.0 );
}

//	fragment-shader
