"""
A simple test script to apply model-view-projection matrix with and
without relative-to-center technique to verify that the results are
equivalent.
"""

import numpy as np

#
# Scene setup (page 161 of the book)
#

# model matrix
M = np.identity(4, dtype=np.float64)

# view matrix
V = np.array([
    [0.78,  0.63,  0.00, -4946218.10],
    [0.20, -0.25,  0.95, -1304368.35],
    [0.60, -0.73, -0.32, -3810548.19],
    [0.00,  0.00,  0.00,        1.00]], dtype=np.float64)

P = np.array([
    [2.80, 0.00,  0.00,  0.00],
    [0.00, 3.73,  0.00,  0.00],
    [0.00, 0.00, -1.00, -0.02],
    [0.00, 0.00, -1.00,  0.00]], dtype=np.float64)

pt_wgs84 = np.array([[6378137.0], [0.0], [0.0], [1.0]], dtype=np.float64)

center_wgs84 = pt_wgs84

#
# Standard MVP (using 64-bit floats)
#

MVP = P @ V @ M

pt_x = MVP @ pt_wgs84
pt_x /= pt_x[3]
print("Basline:\n", pt_x)

#
# MVP with relative-to-center (RTC)
# (pages 164-167)
#

pt_center = pt_wgs84 - center_wgs84
pt_center[3,0] = 1.0   # keep homogenous coordinates

# step 1: calculate center_eye using original MV matrix
MV = V @ M
center_eye = MV @ center_wgs84

# step 2: update fourth column in MV matrix
MV_RTC = MV.copy()
MV_RTC[0, 3] = center_eye[0,0]
MV_RTC[1, 3] = center_eye[1,0]
MV_RTC[2, 3] = center_eye[2,0]

# step 3: calculate final MVP matrix
MVP_RTC = P @ MV_RTC

pt_y = MVP_RTC @ pt_center
pt_y /= pt_y[3]
print("RTC:\n", pt_y)


#
# MVP with RTC - "simplified"
#

# step 1: add translation to the model matrix
M_tr = M.copy()
M_tr[0, 3] = center_wgs84[0,0]
M_tr[1, 3] = center_wgs84[1,0]
M_tr[2, 3] = center_wgs84[2,0]

# step 2: calculate final MVP matrix
MVP_RTC_2 = P @ V @ M_tr

pt_z = MVP_RTC_2 @ pt_center
pt_z /= pt_z[3]
print("RTC simplified:\n", pt_z)
