<?php

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_tester.php                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/22 22:33:58 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/22 22:33:58 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

header("Content-Type: text/plain");

$query_string = $_SERVER['QUERY_STRING'];
if (!empty($query_string)) {
    echo $query_string;
}

$body = file_get_contents('php://input');
if (!empty($body)) {
    echo $body;
}
?>
