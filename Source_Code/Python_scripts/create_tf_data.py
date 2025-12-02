# ML_Training_Code/create_tf_data.py

import os
import glob
import pandas as pd
import xml.etree.ElementTree as ET

def xml_to_csv(path):
    """
    Iterates through all .xml files (annotations) in a given directory and 
    combines the data into a single Pandas DataFrame.
    """
    xml_list = []
    # Find all XML files in the specified path
    for xml_file in glob.glob(os.path.join(path, '*.xml')):
        try:
            tree = ET.parse(xml_file)
            root = tree.getroot()
            
            # Extract basic image info
            filename = root.find('filename').text
            width = int(root.find('size/width').text)
            height = int(root.find('size/height').text)
            
            # Loop through all object annotations in the file
            for member in root.findall('object'):
                value = (filename,
                         width,
                         height,
                         member.find('name').text,  # Class label (e.g., 'human', 'pet')
                         int(member.find('bndbox/xmin').text),
                         int(member.find('bndbox/ymin').text),
                         int(member.find('bndbox/xmax').text),
                         int(member.find('bndbox/ymax').text)
                        )
                xml_list.append(value)
        except Exception as e:
            # This handles errors like files being corrupt or missing required tags
            print(f"Error processing {xml_file}: {e}")
            continue 
    
    column_name = ['filename', 'width', 'height', 'class', 'xmin', 'ymin', 'xmax', 'ymax']
    xml_df = pd.DataFrame(xml_list, columns=column_name)
    return xml_df

# --- Main Execution ---
def main():
    # Get the absolute path to the directory containing this script (ML_Training_Code)
    current_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Construct the absolute path to the ML_Data folder
    base_data_path = os.path.join(current_dir, '..', 'ML_Data') 
    
    # 1. Process Training Data
    train_xml_path = os.path.join(base_data_path, 'train')
    print(f"Looking for train XMLs in: {train_xml_path}") # CHECK THIS PATH
    train_df = xml_to_csv(train_xml_path)
    train_df.to_csv('train_labels.csv', index=None)
    print('Successfully created train_labels.csv')

    # 2. Process Testing Data
    test_xml_path = os.path.join(base_data_path, 'test')
    print(f"Looking for test XMLs in: {test_xml_path}") # CHECK THIS PATH
    test_df = xml_to_csv(test_xml_path)
    test_df.to_csv('test_labels.csv', index=None)
    print('Successfully created test_labels.csv')

if __name__ == '__main__':
    main()