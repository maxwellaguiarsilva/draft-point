//	unit-ray triangle evaluator using precomputed centroid-relative corners and unit normal
hit_result evaluate_triangle( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
	vec3	normal = vec3_from( figure_item.parameters, 9u );
	float	denominator = dot( unit_direction, normal );
	if( abs( denominator ) <= 1e-6 )
		return	no_hit( );
	vec3	local_center = figure_item.position - ray_origin;
	float	hit_distance = dot( local_center, normal ) / denominator;
	vec3	hit_local = hit_distance * unit_direction - local_center;
	vec3	corner_0 = vec3_from( figure_item.parameters, 0u ) - hit_local;
	vec3	corner_1 = vec3_from( figure_item.parameters, 3u ) - hit_local;
	vec3	corner_2 = vec3_from( figure_item.parameters, 6u ) - hit_local;
	if( min_axis( vec3(
		 dot( cross( corner_1, corner_2 ), normal )
		,dot( cross( corner_2, corner_0 ), normal )
		,dot( cross( corner_0, corner_1 ), normal )
	) ) < 0.0 )
		return	no_hit( );
	return	hit_result( true, hit_distance, abs( denominator ) );
}
