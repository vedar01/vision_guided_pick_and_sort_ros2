from setuptools import find_packages, setup

package_name = 'pick_n_place_py'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='vedar01',
    maintainer_email='vedar01@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            "obj_to_coords = pick_n_place_py.obj_detection_to_coords:main",
            "pixel_to_real_coords = pick_n_place_py.pixel_to_real_coords:main",
            "clrd_obj_coords = pick_n_place_py.clrd_obj_detection:main"
        ],
    },
)
