#  ************************************************************************** */
#                                                                             */
#                                                         :::      ::::::::   */
#    upload.py                                     :+:      :+:    :+:   */
#                                                     +:+ +:+         +:+     */
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
#                                                 +#+#+#+#+#+   +#+           */
#    Created: 2024/03/30 13:24:02 by tchoquet          #+#    #+#             */
#                                                     ###   ########.fr       */
#                                                                             */
#  ************************************************************************** */

import cgi
import os

# Set the upload directory
upload_dir = "www/uploads"

# Create the upload directory if it doesn't exist
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir)

# Create a form instance
form = cgi.FieldStorage()

# Check if any files were uploaded
if len(form) > 0:
    # Convert the form object into a list
    form_list = list(form)

    # Iterate over all the items in the form
    for item in form_list:
        # Check if the item is a file
        if form[item].filename:
            # Get the filename
            filename = os.path.basename(form[item].filename)

            # Set the path to save the file
            filepath = os.path.join(upload_dir, filename)

            # Save the file
            with open(filepath, "wb") as f:
                f.write(form[item].file.read())

    # Print a success message
    print("Content-Type: text/html\r")
    print("\r")
    print("<h1>Files uploaded successfully!</h1>\r")
else:
    # Print an error message
    print("Content-Type: text/html\r")
    print("\r")
    print("<h1>Error: No files uploaded.</h1>\r")
